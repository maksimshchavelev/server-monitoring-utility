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
Json::Value smu_server::Application::collect_metrics() {
    // If the module's poll ratio is greater than 1, the last get_data call must be cached,
    // otherwise the module data will not reach the user.
    static std::unordered_map<std::string_view /* module_name */, Json::Value /* cached_data */>
        poll_ratio_between_get_datas_cache;


    Json::Value root;


    std::lock_guard<std::mutex> lock(m_modules_mutex);
    for (const auto& module : m_modules) {
        // Skip module if module is disabled
        if (!module->is_enabled()) {
            continue;
        }

        try {
            // Need to cache
            if (module->get_poll_ratio() == 0) {
                // Try to cache if not cached
                if (auto iter = m_module_cache.find(module->module_name());
                    iter == m_module_cache.end()) {

                    // Cache first call of `module->get_data()`
                    if (auto module_data = module->get_data(); module_data.has_value()) {
                        m_module_cache[module->module_name()] = std::move(module_data.value());
                    } else {
                        // Failed to cache
                        logger().log_warning(
                            std::format("Failed to cache data from module {} (marked as cacheable)",
                                        module->module_name()));
                    }
                }

                // Load cache
                root[module->module_name().data()] =
                    m_module_cache[module->module_name()]; // There is no `std::move`, as this would
                                                           // otherwise invalidate the cache.
                continue;
            }

            // Check necessity of polling uncacheable module
            if (module->m_poll_counter >= module->get_poll_ratio() - 1) {
                // Poll uncacheable module
                if (auto module_data = module->get_data(); module_data.has_value()) {
                    // Do not move to cache if poll ratio is 1
                    if (module->get_poll_ratio() == 1) {
                        root[module->module_name().data()] = std::move(module_data.value());
                    } else {
                        // Otherwise, first to the cache, then to `root`
                        poll_ratio_between_get_datas_cache[module->module_name()] =
                            module_data.value();
                        root[module->module_name().data()] = std::move(module_data.value());
                    }

                    module->m_poll_counter = 0; // reset poll counter
                    continue;
                }
            } else {
                // Load from cache instead of calling `get_data` if no necessity
                root[module->module_name().data()] =
                    poll_ratio_between_get_datas_cache[module->module_name()];
            }

            ++module->m_poll_counter; // increase poll counter

        } catch (const std::exception& e) {
            logger().log_warning(std::format(
                "Failed to get data from module {}, cause: {}", module->module_name(), e.what()));
        }
    }

    return root;
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
            logger().log_error(std::format("Can't load dynamic module '{}', cause: {}",
                                           module_name,
                                           module.error()));
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
