/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.cpp
 * @brief Implementation of the Application class
 */


#include "core/internals/app.hpp"
#include "compile-time_config.hpp"
#include "logger/logger.hpp"
#include "version.hpp"
#include <cxxopts.hpp>
#include <external_module_loader/external_module_loader.hpp>
#include <filesystem>
#include <sys/stat.h>


// Public constructor
smu_server::Application& smu_server::Application::instance() {
    static Application app;
    return app;
}




// Public method
void smu_server::Application::add_module_to_queue(
    std::function<void(Application&)> register_function) {
    m_modules_queue.push_back(std::move(register_function));
}




// Public method
void smu_server::Application::run(int argc, char** argv) {
    if (parse_argv(argc, argv)) {
        m_need_save_config_in_destructor = false;
        return; // Exit server without running
    }

    for (const auto& module_registrar : m_modules_queue) {
        module_registrar(*this); // register each built-in module
    }

    // Register dynamic modules
    register_dynamic_modules(MODULES_CONFIGS_DIR);

    // Get port
    auto port = m_server_config.get<int>("port");
    if (!port.has_value()) {
        throw std::runtime_error("Can't get port to running!");
    }

    // Run CLI
    m_cli.run();

    // Run network
    m_network.run(static_cast<uint16_t>(port.value()), [this]() { run_sending_metrics_async(); });
}




// Public method
std::vector<uint8_t> smu_server::Application::collect_metrics() {
    // cache for the "between polls" case (poll_ratio > 1)
    static std::unordered_map<std::string, std::vector<uint8_t>> poll_ratio_between_get_datas_cache;

    // final frame: we'll build header (5 bytes) and then append payload
    std::vector<uint8_t> frame;
    frame.resize(5, 0x00); // placeholder for [version(1)] [frame_payload_size(4)]

    // payload = concatenation of container blocks from modules (each container includes its own
    // header)
    std::vector<uint8_t> payload;
    payload.reserve(1024); // heuristic; optional

    // helper: extract module's "container block" from module_frame bytes and append to 'payload'.
    // If module_frame is a full MDTP frame (declared_size matches), copy bytes [5 ..
    // 5+declared_size-1]. Otherwise fallback: append whole module_frame (assume it's already a
    // container block).
    auto append_module_block = [&](const std::vector<uint8_t>& module_frame) {
        if (module_frame.size() >= 5) {
            uint32_t declared = internals::read_uint32_be(module_frame, 1);
            // sanity: declared shouldn't overflow the available bytes
            if (module_frame.size() >= static_cast<size_t>(5 + declared)) {
                payload.insert(
                    payload.end(), module_frame.begin() + 5, module_frame.begin() + 5 + declared);
                return;
            }
        }
        // fallback (non-framed data / already a container block)
        payload.insert(payload.end(), module_frame.begin(), module_frame.end());
    };

    std::lock_guard<std::mutex> lock(m_modules_mutex);

    for (const auto& module_ptr : m_modules) {
        auto& module = *module_ptr;

        // skip disabled modules
        if (!module.is_enabled())
            continue;

        const std::string name{module.module_name()}; // stable key for maps

        try {
            const uint32_t poll_ratio = module.get_poll_ratio();

            // ---- cacheable modules: poll_ratio == 0 ----
            if (poll_ratio == 0) {
                auto it_cache = m_module_cache.find(name);
                if (it_cache == m_module_cache.end()) {
                    // try to get and cache once
                    if (auto module_data = module.get_data(); module_data.has_value()) {
                        m_module_cache[name] = std::move(module_data.value());
                    } else {
                        logger().log_warning(std::format(
                            "Failed to cache data from module {} (marked as cacheable)", name));
                        continue; // skip this module for this cycle
                    }
                }
                // append cached block (if any)
                if (!m_module_cache[name].empty()) {
                    append_module_block(m_module_cache[name]);
                }
                continue;
            }

            // ---- pollable modules: poll_ratio >= 1 ----
            if (module.m_poll_counter >= (poll_ratio > 0 ? poll_ratio - 1 : 0)) {
                // time to poll this module
                if (auto module_data = module.get_data(); module_data.has_value()) {
                    auto bytes = std::move(module_data.value());
                    if (poll_ratio == 1) {
                        // always fresh, do not store in between-cache
                        append_module_block(bytes);
                    } else {
                        // store last polled value for between-polls usage
                        poll_ratio_between_get_datas_cache[name] = bytes;
                        append_module_block(poll_ratio_between_get_datas_cache[name]);
                    }
                    module.m_poll_counter = 0;
                    continue;
                } else {
                    logger().log_warning(
                        std::format("Failed to poll module {} when scheduled", name));
                    // if we have a previously polled value, append it; otherwise skip
                    auto it = poll_ratio_between_get_datas_cache.find(name);
                    if (it != poll_ratio_between_get_datas_cache.end()) {
                        append_module_block(it->second);
                    }
                    // reset counter? keep it unchanged to try again next cycle
                }
            } else {
                // not time to poll: try to use the 'between polls' cache if present
                auto it = poll_ratio_between_get_datas_cache.find(name);
                if (it != poll_ratio_between_get_datas_cache.end()) {
                    append_module_block(it->second);
                } else {
                    // nothing to append — skip
                }
            }

            // increment poll counter for next cycle
            ++module.m_poll_counter;

        } catch (const std::exception& e) {
            logger().log_warning(std::format(
                "Failed to get data from module {}, cause: {}", module.module_name(), e.what()));
            // skip module on error
        }
    } // end for modules

    // Write final frame header:
    // - version
    frame[0] = static_cast<uint8_t>(MDTP_VERSION);
    // - frame payload size = total bytes after the frame header (i.e. payload.size())
    uint32_t frame_payload_size = static_cast<uint32_t>(payload.size());
    internals::write_uint32_be(frame, 1, frame_payload_size);

    // append payload (concatenated container blocks)
    if (!payload.empty()) {
        frame.insert(frame.end(), payload.begin(), payload.end());
    }

    return frame;
}




// Public method
void smu_server::Application::run_sending_metrics_async() {
    static bool running{false};

    if (running) {
        logger().log_warning(
            "Application::run_sending_metrics_async() is already running. Skipping run "
            "again request");
        return;
    }

    running = true;

    unsigned int send_interval = m_server_config.get<unsigned int>("send_interval_ms").value();
    std::thread  runner([this, send_interval]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                send_interval)); // sleep for `sleep_interval_ms` milliseconds

            if (m_network.get_connections_count() > 0) {
                auto metrics = collect_metrics();
                if (!metrics.empty()) {
                    // If metrics are empty
                    m_network.send_everyone(metrics);
                }
            }
        }
    });

    runner.detach();
}




// Public method
void smu_server::Application::save_configs() const noexcept {
    auto& manager = Config_IO::instance();

    // Saving server configuration
    if (auto res = manager.save_server_config(m_server_config); !res.has_value()) {
        // If error
        logger().log_error(std::format(
            "\033[31mError saving server configuration (cause: {})\033[0m", res.error()));
    }

    // Saving module configurations
    for (const auto& module : m_modules) {
        if (auto res =
                manager.save_module_config(module->module_name(), module->get_configuration());
            !res.has_value()) {
            // If error
            logger().log_error(std::format(
                "\033[31mError saving configuration of module \"{}\" (cause: {}\033[0m)",
                module->module_name(),
                res.error()));
        }
    }
}




// Private constructor
smu_server::Application::Application() :
    m_server_config(Config_IO::instance().get_server_config()), m_cli(*this) {}




// Private destructor
smu_server::Application::~Application() {
    if (m_need_save_config_in_destructor) {
        save_configs();
    }
}




// Private method
void smu_server::Application::register_dynamic_modules(const std::string_view modules_directory) {
    std::lock_guard<std::mutex> lock(m_modules_mutex);

    std::filesystem::directory_entry entry(modules_directory);

    for (const auto& dir : std::filesystem::directory_iterator(entry)) {
        // If not
        if (!dir.is_directory()) {
            continue;
        }

        auto module_name = dir.path().filename().string();
        auto so_file = dir.path() / (module_name + ".so");

        // Directory without .so module
        if (!std::filesystem::exists(so_file)) {
            continue;
        }

        // If incorrect permissions (must be r-x------)
        if (auto perms = std::filesystem::status(so_file).permissions();
            perms != (std::filesystem::perms::owner_read | std::filesystem::perms::owner_exec)) {
            logger().log_error(std::format(
                "Incorrect permissions of file {} (module {}). Permissions must be 'r-x------'",
                so_file.string(),
                module_name));
            continue;
        }

        // If incorrect owner (must be root)
        {
            struct stat st;
            if (stat(so_file.c_str(), &st) != 0) {
                logger().log_error(std::format("'stat' syscal error, cause: {}", strerror(errno)));
                continue;
            }

            // If owner isn't root
            if (st.st_uid != 0) {
                logger().log_error(
                    std::format("Owner of file {} must be 'root'!", so_file.c_str()));
                continue;
            }
        }

        // Load config
        auto config = Config_IO::instance().get_module_config(module_name);

        // If error
        if (config.empty()) {
            logger().log_error(
                std::format("Failed to open config of dynamic module '{}'", module_name));
            continue;
        }

        if (auto module = ExternalModuleLoader::load(so_file.c_str(), config); module.has_value()) {
            logger().log_success(std::format("The dynamic module named '{}' (description: {}) was "
                                             "successfully loaded from file {}",
                                             module_name,
                                             module.value()->module_description(),
                                             so_file.string()));
            m_modules.push_back(std::move(module.value()));
        } else {
            logger().log_error(std::format(
                "Can't load dynamic module '{}', cause: {}", module_name, module.error()));
            continue;
        }
    }
}




// Private method
bool smu_server::Application::parse_argv(int argc, char** argv) const noexcept {
    cxxopts::Options options("smu-server", "Server part included in server-monitoring-utility");

    options.add_options()("version", "Show smu-server version")("h,help", "Show help information");

    cxxopts::ParseResult result;

    try {
        result = options.parse(argc, argv);
    } catch (std::exception& e) {
        std::cout << "Argument parsing error: " << e.what() << std::endl;
        return true; // exit
    }

    // --version
    if (result.contains("version")) {
        std::cout << std::format("smu-server version is {}.{}.{}",
                                 PROJECT_VERSION_MAJOR,
                                 PROJECT_VERSION_MINOR,
                                 PROJECT_VERSION_PATCH)
                  << std::endl;
        return true; // exit
    }
    // --help
    else if (result.contains("help") || result.contains("h")) {
        std::cout << options.help();
        return true; // exit
    }

    return false;
}
