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
    // Between-poll cache for poll_ratio > 1
    static std::unordered_map<std::string, std::vector<uint8_t>> poll_ratio_between_get_datas_cache;

    // Final MDTP frame: [version:1][payload_size:4] + payload
    std::vector<uint8_t> frame(5, 0x00);
    std::vector<uint8_t> payload; // concatenation of per-module named containers

    // Helper: wrap module_frame into a named container with module_name,
    // stripping the module's own MDTP frame header (5 bytes).
    auto append_module_as_named_container = [&](const std::vector<uint8_t>& module_frame,
                                                const std::string&          module_name) {
        // Expect a full MDTP frame: at least 5 bytes
        if (module_frame.size() < 5) {
            logger().log_warning(
                std::format("Module '{}' returned too small frame ({} bytes) — skip",
                            module_name,
                            module_frame.size()));
            return;
        }

        // Read declared payload size from module-frame header
        const uint32_t inner_size = internals::read_uint32_be(module_frame, 1);
        const size_t   need = static_cast<size_t>(5) + static_cast<size_t>(inner_size);

        if (module_frame.size() < need) {
            logger().log_warning(
                std::format("Module '{}' returned truncated frame: declared={}, actual={} — skip",
                            module_name,
                            inner_size,
                            module_frame.size()));
            return;
        }

        // The module payload is everything after its 5-byte header
        const uint8_t* inner_begin = module_frame.data() + 5;

        // Build container header:
        // [node type=0:1][name len:4][name:bytes][payload size:4][payload...]
        const uint32_t name_len = static_cast<uint32_t>(module_name.size());
        const size_t   header = 1 + 4 + name_len + 4;
        const size_t   old_size = payload.size();

        payload.resize(old_size + header + inner_size);

        size_t off = old_size;

        // node type = 0 (container)
        payload[off++] = 0;

        // node name length (BE)
        internals::write_uint32_be(payload, off, name_len);
        off += 4;

        // node name bytes (no terminating zero)
        std::memcpy(payload.data() + off, module_name.data(), module_name.size());
        off += module_name.size();

        // payload size (BE) — equal to module's inner payload size
        internals::write_uint32_be(payload, off, inner_size);
        off += 4;

        // payload bytes (module payload without frame header)
        std::memcpy(payload.data() + off, inner_begin, inner_size);
        // off += inner_size; // not required further
    };

    std::lock_guard<std::mutex> lock(m_modules_mutex);

    for (const auto& mod_ptr : m_modules) {
        if (!mod_ptr)
            continue;
        auto& module = *mod_ptr;

        if (!module.is_enabled())
            continue;

        const std::string name{module.module_name()};
        try {
            const uint32_t poll_ratio = module.get_poll_ratio();

            // Cache-once modules (poll_ratio == 0)
            if (poll_ratio == 0) {
                auto it = m_module_cache.find(name);
                if (it == m_module_cache.end()) {
                    if (auto module_data = module.get_data(); module_data.has_value()) {
                        // Store full module frame
                        m_module_cache[name] = std::move(module_data.value());
                    } else {
                        logger().log_warning(
                            std::format("Failed to cache data from module '{}' (cacheable)", name));
                        continue;
                    }
                }
                // Wrap cached frame into named container
                append_module_as_named_container(m_module_cache[name], name);
                continue;
            }

            // Pollable modules (poll_ratio >= 1)
            if (module.m_poll_counter >= (poll_ratio > 0 ? poll_ratio - 1 : 0)) {
                // Time to poll
                if (auto module_data = module.get_data(); module_data.has_value()) {
                    auto bytes = std::move(module_data.value()); // full module frame
                    if (poll_ratio == 1) {
                        append_module_as_named_container(bytes, name);
                    } else {
                        // Keep last result for between-polls
                        poll_ratio_between_get_datas_cache[name] = std::move(bytes);
                        append_module_as_named_container(poll_ratio_between_get_datas_cache[name],
                                                         name);
                    }
                    module.m_poll_counter = 0;
                    continue;
                } else {
                    logger().log_warning(
                        std::format("Failed to poll module '{}' when scheduled", name));
                    // Fallback to between-polls cache if exists
                    auto it = poll_ratio_between_get_datas_cache.find(name);
                    if (it != poll_ratio_between_get_datas_cache.end()) {
                        append_module_as_named_container(it->second, name);
                    }
                }
            } else {
                // Not time to poll: try between-polls cache
                auto it = poll_ratio_between_get_datas_cache.find(name);
                if (it != poll_ratio_between_get_datas_cache.end()) {
                    append_module_as_named_container(it->second, name);
                }
            }

            ++module.m_poll_counter;

        } catch (const std::exception& e) {
            logger().log_warning(
                std::format("Failed to get data from module '{}', cause: {}", name, e.what()));
        }
    }

    // Write final frame header
    frame[0] = static_cast<uint8_t>(MDTP_VERSION);
    internals::write_uint32_be(frame, 1, static_cast<uint32_t>(payload.size()));

    // Append payload
    frame.insert(frame.end(), payload.begin(), payload.end());
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

        // Log registration beginning
        logger().log_colorless(std::format("Registering an external module with name "
                                           "\"\033[36m{}\033[0m\"...",
                                           module_name));

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
            logger().log_warning(std::format(
                "Failed to open config of dynamic module '{}'. Continuing with default values",
                module_name));
        }

        if (auto module = ExternalModuleLoader::load(so_file.c_str(), config); module.has_value()) {
            // Module status (RUNNING / STOPPED)
            const char* status_string =
                module.value()->is_enabled() ? "\033[32mRUNNING\033[0m" : "\033[31mSTOPPED\033[0m";
            // Log success
            logger().log_colorless(
                std::format("[MODULE \033[36m{}\033[0m "
                            "(\"\033[36m{}\033[0m\")] \033[32mRegistered\033[0m ({})\n",
                            module.value()->module_name(),
                            module.value()->module_description(),
                            status_string));
            // Append module
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
