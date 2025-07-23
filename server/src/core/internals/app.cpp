/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/app.cpp
 * @brief Implementation of the Application class
 */


#include "core/internals/app.hpp"
#include "compile-time_config.hpp"
#include "version.hpp"
#include <cxxopts.hpp>


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
        module_registrar(*this); // register each module
    }

    // Init heavy server objects
    m_main_ws_controller_ptr.emplace(std::make_shared<MainWebsocketController>());
    m_ipc.emplace(IPC(ABSTRACT_SOCKET_NAME));

    // Proceed commands from CLI
    m_ipc.value().run([&](const IPC::Command cmd, const std::vector<std::string>& args) {
        return ipc_command_receiver(cmd, args);
    });

    // Get port from config
    uint16_t port = static_cast<uint16_t>(m_server_config["port"].asUInt());

    drogon::app().addListener("0.0.0.0", port).registerController(m_main_ws_controller_ptr.value());
    drogon::app().getLoop()->runAfter(0.0, [this]() { run_sending_metrics_async(); });
    drogon::app().run();
}




// Public method
Json::Value smu_server::Application::collect_metrics() {
    Json::Value root;

    std::lock_guard<std::mutex> lock(m_modules_mutex);
    for (const auto& module : m_modules) {
        // Skip module if module is disabled
        if(!module->is_enabled()) {
            continue;
        }

        if (auto module_data = module->get_data(); module_data.has_value()) {
            root[module->module_name().data()] = std::move(module_data.value());
        }
    }

    return root;
}




// Public method
void smu_server::Application::run_sending_metrics_async() {
    static bool running{false};

    if (running) {
        LOG_WARN << "Application::run_sending_metrics_async() is already running. Skipping run "
                    "again request";
        return;
    }

    running = true;

    unsigned int send_interval = m_server_config["send_interval_ms"].asUInt();
    std::thread  runner([this, send_interval]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                send_interval)); // sleep for `sleep_interval_ms` milliseconds

            if (m_main_ws_controller_ptr.value()->get_connections_count() > 0) {
                auto metrics = collect_metrics();
                if (!metrics.empty()) {
                    // If metrics are empty
                    m_main_ws_controller_ptr.value()->send_everyone(metrics);
                }
            }
        }
    });

    runner.detach();
}




// Public method
void smu_server::Application::save_configs() const noexcept {
    auto& manager = ConfigManager::instance();

    // Saving server configuration
    if (auto res = manager.save_server_config(); !res.has_value()) {
        // If error
        LOG_ERROR << std::format("\033[31mError saving server configuration (cause: {})\033[0m",
                                 res.error());
    }

    // Saving module configurations
    for (const auto& module : m_modules) {
        if (auto res =
                manager.save_module_config(module->module_name(), module->get_configuration());
            !res.has_value()) {
            // If error
            LOG_ERROR << std::format(
                "\033[31mError saving configuration of module \"{}\" (cause: {}\033[0m)",
                module->module_name(),
                res.error());
        }
    }
}




// Private constructor
smu_server::Application::Application() :
    m_server_config(ConfigManager::instance().get_server_config()) {}




// Private destructor
smu_server::Application::~Application() {
    if (m_need_save_config_in_destructor) {
        save_configs();
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




// ================================ FOR CLI COMMANDS ================================


// Private method
std::string smu_server::Application::ipc_command_receiver(const IPC::Command              cmd,
                                                          const std::vector<std::string>& args) {
    // --list <args>
    if (cmd == IPC::Command::LIST) {
        // --list modules
        if (args[0] == "modules") {
            return list_modules();
        }
    }


    // --run <args>
    if (cmd == IPC::Command::RUN) {
        // --run <modules>
        for (const auto& module_name : args) {

            if (auto iter = std::find_if(
                    m_modules.begin(),
                    m_modules.end(),
                    [&](const auto& module) { return module->module_name() == module_name; });
                iter != m_modules.end()) {

                // If found module with name `module_name`
                (*iter)->enable();
                return "\033[32mDone!\033[0m";

            } else {
                // Return red error
                return std::format("\033[31mModule with name {} doesn't exists!\033[0m",
                                   module_name);
            }
        }
    }


    // --stop <args>
    if (cmd == IPC::Command::STOP) {
        // --run <modules>
        for (const auto& module_name : args) {

            if (auto iter = std::find_if(
                    m_modules.begin(),
                    m_modules.end(),
                    [&](const auto& module) { return module->module_name() == module_name; });
                iter != m_modules.end()) {

                // If found module with name `module_name`
                (*iter)->disable();
                return "\033[32mDone!\033[0m";

            } else {
                // Return red error
                return std::format("\033[31mModule with name {} doesn't exists!\033[0m",
                                   module_name);
            }
        }
    }

    return "Invalid syntax";
}




// Private method
std::string smu_server::Application::list_modules() const {
    std::string result = "NAME\t\tSTATUS\t\tDESCRIPTION\n";

    for (const auto& module : m_modules) {
        std::string current_module_info(1, '\n');

        // Module name
        current_module_info.append(module->module_name());

        // Tab
        current_module_info.append("\t\t");

        // Status
        if (module->is_enabled()) {
            // Print green module name
            current_module_info.append("\033[32mRUNNING\033[0m");
        } else {
            // Print red module name
            current_module_info.append("\033[31mSTOPPED\033[0m");
        }

        // Tab
        current_module_info.append("\t\t");

        // Description
        current_module_info.append(module->module_description());

        result.append(current_module_info);
    }

    return result;
}
