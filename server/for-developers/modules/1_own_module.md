# Developing Your Own Module
This section will teach you how to develop your own module for smu-server.

> ⚠️ This guide does not fully adhere to the codestyle (in terms of comments and indentation) adopted in this project. See the source code of actual files <tt>ram.hpp</tt> and <tt>ram.cpp</tt> for examples.

# Getting Started. General Concept
Modules act as an intermediary layer between the server and the operating system - they collect data, process it, form JSON, and pass it to the server for further delivery to the client. Here are some requirements for a module:

- The module should not consume excessive resources (no unnecessary "background" computations)
- The module should not block the thread when attempting to retrieve data from it
- The module can run in a separate thread (e.g., when calculating average values over time), but must properly stop when `disable()` is called and start when `enable()` is called
- The module should not create any files (read-only operations only)
- The JSON response structure with data should be logical (include metric groups if metrics need semantic separation, with appropriate and correct naming)
- The module class must reside in the `smu_server` namespace
- Place the module header file in `include/modules/<module name>/` and the source file in `src/modules/<module name>/`
- The module name (class name) must not contain `Module` in any case. The module name is displayed in the client-side UI.

When designing smu-server, I envisioned functionality that would maximize simplicity and standardization for module creation. We will explore this functionality next.

# IModule - Where It All Begins
All modules must inherit from the `IModule` class (otherwise it will cause a compilation error). This is a base abstract class that provides essential functionality. Its structure is shown below:

```cpp
class IModule {
  public:
    IModule(const Config& configuration);
    virtual ~IModule() = default;

    virtual const Config& get_configuration() const noexcept;
    virtual std::optional<Json::Value> get_data() = 0;
    virtual void enable();
    virtual void disable();
    virtual bool is_enabled() const;
    
    virtual void set_poll_ratio(uint32_t poll_ratio);
    virtual uint32_t get_poll_ratio() const;
    
    virtual constexpr std::string_view module_name() const noexcept = 0;
    virtual constexpr std::string_view module_description() const noexcept = 0;



  protected:
    Config m_configuration;
    bool   m_enabled{false};

    uint32_t m_poll_ratio{1};

    enum class LogType { INFO, WARNING, ERROR };

    void log(LogType log_type, const std::string_view message) const;


  private:
    // You don't need to know what's here :)
};
```

Let's examine it.

### Constructor
Accepts `const Config&` (see `smu_server::Config` documentation) with the module configuration loaded from the corresponding file (see User Guide). 

> ⚠️ If `configuration` is empty, the module must generate its own configuration and populate `m_configuration`. This configuration will be saved to a file and loaded on subsequent launches.

> ⚠️ If a fatal error occurs, the constructor **must throw an exception** and then the module **will not be registered** in the server core!


### get_configuration
Returns the module's current configuration. Used for saving configuration on application exit.


### get_data
Forms a JSON with data and returns it. Returns `std::nullopt` in case of an error. We'll explore this in more detail in the practical section. Implement this method yourself.


### enable() and disable()
Enable and disable the module. You don't need to implement these methods (unless you have custom enable/disable logic). By default, they modify the `m_enabled` field.


### is_enabled
Returns whether the module is enabled or not. You don't need to implement this method (unless you have custom logic). By default, returns the value of the `m_enabled` field.


### set_poll_ratio and get_poll_ratio
Set the poll ratio and return it accordingly. The poll ratio regulates the frequency of module polling. For example, if the poll ratio is 5, the module will be polled by the server core every 5 polling cycles. Thus, if the global polling interval is 1 second, the module will be polled every 5 seconds. This is necessary to avoid unnecessary polling if the module data is not updated that often.

> You can set the poll ratio to 0, in which case the server core will cache the data received on the first call to `get_data` and `get_data` will not be called again. This applies to [modules with static data](2_static_data_modules.md).


### module_name and module_description
Methods to get the module's name and description. **Never implement these yourself**, as they are properly implemented via the `REGISTER_MODULE` macro, which we'll discuss next.


### log
This function is needed to log information, warnings or errors.
> ⚠️ Use it instead of `std::cout`! `log` method is **fully thread-safe**

Accepts LogType as the message type. The following values give these colors:
- `LogType::INFO` - **white** message
- `LogType::INFO` - **yellow** message
- `LogType::ERROR` - **red** message

Example usage:
```cpp
log(LogType::ERROR, "Error opening file");
```

> Use `LogType::INFO` for informational messages, `LogType::WARNING` for warnings for situations that *don't affect the module much* (it can work) and `LogType::ERROR` for fatal errors.
 
 

## Practical Section
Let's create a RAM module that provides information about RAM and swap usage. Create two files: `include/modules/ram/ram.hpp` and `src/modules/ram/ram.cpp`.

Start with `ram.hpp`:
First, add the file description and copyright:

@verbatim
/**
 * @file modules/ram/ram.hpp
 * @brief File with RAM module class
 *
 * GPLv3 LICENSE, Copyright (C) 2025, Your name <Your email>
 * See LICENSE for details.
 */
@endverbatim


> ⚠️ The _GPLv3_ license is mandatory for all software components of smu!

Then add `#pragma once` and include the file providing core functionality:

```cpp
#pragma once
#include "core/core.hpp"
```

Next, write the following code:

```cpp
namespace smu_server {

/**
 * @brief A module that allows you to get information about RAM
 * @see `IModule` for methods
 */
class RAM : public IModule {
  public:
    REGISTER_MODULE(RAM, "A module that allows you to get information about RAM")

    RAM(const Config& config);
    std::optional<Json::Value> get_data() override;
    void enable() override;
    void disable() override;
};

} // end of namespace smu_server
```

We inherit publicly from `IModule` and immediately use the `REGISTER_MODULE` macro. Let's examine this macro in detail. It registers the module in the system - without it, the module won't work! This macro eliminates the need to modify other program components to register your module. It takes the module name (must match the class name) and a description that appears in logs during startup. `REGISTER_MODULE` implements the following methods:

- `module_name()` - get module name
- `module_description()` - get module description
- `module_name_static()` - static method to get module name (callable via type: `RAM::module_name_static()`)
- `module_description_static()` - static method to get module description (callable similarly to `module_name_static()`)

> ⚠️ `REGISTER_MODULE` makes the entire following area public, so be careful!

Then we declare the constructor and implement pure virtual methods.
Now let's look at the implementation in `ram.cpp`:

```cpp

/**
 * @file modules/ram/ram.cpp
 * @brief File with RAM module class
 * 
 * @copyright Copyright (©) 2025, Your name <Your email>
 * @license GPLv3 license, See LICENSE for details
 */

#include "modules/ram/ram.hpp"
#include <sys/sysinfo.h>
    
namespace smu_server {

// Public method
RAM::RAM(const Json::Value& config) : IModule(config) {
    // Create new configuration
    if (m_configuration.empty()) {
        m_configuration.set("enabled", true);
    }

    // Check if the `enabled` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto enabled = m_configuration.get<bool>("enabled"); enabled.has_value()) {
        m_enabled = enabled.value();
    } else {
        log(LogType::ERROR, "The 'enabled' field is missing. Can't continue");
        throw std::runtime_error("The 'enabled' field is missing");
    }
}


// Public method
std::optional<Json::Value> RAM::get_data() {
    struct sysinfo info;

    if (sysinfo(&info) == -1) { // error
        return std::nullopt;
    } // else


    auto root = make_root_node(
        make_container_node("RAM info",
            make_value_node("Total RAM", info.totalram / 1024 / 1024, "MB"),
            make_value_node("Used RAM", (info.totalram - info.freeram) / 1024 / 1024, "MB"),
            make_value_node("Used RAM (%)", (info.totalram - info.freeram) * 100 / info.totalram, "%")
        ),

        make_container_node("SWAP info",
            make_value_node("Total SWAP", info.totalswap / 1024 / 1024, "MB"),
            make_value_node("SWAP usage", (info.totalswap - info.freeswap) / 1024 / 1024, "MB"),
            make_value_node("SWAP usage (%)", (info.totalswap - info.freeswap) * 100 / info.totalswap, "%")
        )
    );

    return root->to_json();
}


// Public method
void RAM::enable() {
    m_configuration.set("enabled", true);
    m_enabled = true;
}


// Public method
void RAM::disable() {
    m_configuration.set("enabled", false);
    m_enabled = false;
}

} // namespace smu_server
```

Besides including `ram.hpp`, we include `<sys/sysinfo.h>` which contains the `sysinfo` system call we need.
In the constructor, we read the config and apply settings (here only the `enabled` setting controls module status). If we get an empty config, we configure from scratch.

> ⚠️ `Config::get` returns `std::optional`, always check if it has a value! See `smu_server::Config` documentation

> ⚠️ Never call `enable()` and `disable()` from the constructor if you override them in your module, otherwise the base class implementation will be called instead of your implementation!

For simplicity, this code does not include the `poll ratio` setting, but you can configure it yourself.

> By default, the poll ratio is 1.

Let's focus on `get_data`.

> You do not need to do a check (and, for example, return `std::nullopt`) on the status of the module, since it will not be polled by the server core if it is off, and therefore `get_data` will not be called

We form the JSON with metric values. Let's examine this in detail.
The core idea is to group metrics (the smallest data unit in a module) using ***nodes***. A node can be:

- **Value Node**: Contains a value, unit of measurement, and metric name. `units` can be empty. If the value is empty, the client will display *N/A*. Type: `value`
- **Container Node**: Can contain value nodes and other containers. Has a name. Used for semantic grouping of metrics (creates a tree structure). Type: `container`
- **Root Node**: The top-level node that can contain metrics and containers. Has no name or type. **Important: The root node must always be present. Never send containers or metrics without wrapping them in a root node!**

> Node handling functions are located in `core/core.hpp` (already included)

Create nodes using:  
- `make_root_node()` (accepts any number of child nodes)  
- `make_container_node()` (first specify name, then list child nodes)  
- `make_value_node()` (specify name, value, and units)

> Values passed to `value_node` can be numbers or strings (automatically converted to strings if needed).

Code example:

```cpp
auto root = make_root_node(
       make_value_node("RAM usage", 4217, "MB")
);
```
Creates a root node containing a value node named "RAM usage" with value 4217 and unit "MB".

> Node names, values, and units are displayed in the client UI. Use clear and appropriate names. For units, use common abbreviations. Node names should be concise and unambiguous.

This code produces the following JSON:

```json
"RAM usage": {
     "type": "value",
     "units": "MB",
     "value": "4217"
}
```
Convert a node to JSON using `root->to_json()` (only call this on the root node!).  

Another example:

```cpp
auto root = make_root_node(
    make_container_node("Simple container",
        make_value_node("Simple value 1", 12345, "bytes"),
        make_value_node("Simple value 2", 4, "GB")
    ),
    make_value_node("RAM usage", 4217, "MB")
);
```
Creates a root node containing a container (with two metrics) and a standalone metric. Resulting JSON:

```json
"RAM": {
    "RAM usage": {
        "type": "value",
        "units": "MB",
        "value": "4217"
    },
    "Simple container": {
        "Simple value 1": {
            "type": "value",
            "units": "bytes",
            "value": "12345"
        },
        "Simple value 2": {
            "type": "value",
            "units": "GB",
            "value": "42"
        },     
        "type": "container"
    }
}
```
Now you understand the code in `ram.cpp`. For reference, this module's JSON output (on a test PC) looks like:

```json
"RAM info": {
    "Total RAM": {
        "type": "value",
        "units": "MB",
        "value": "32018"
    },
    "Used RAM": {
        "type": "value",
        "units": "MB",
        "value": "5958"
    },
    "Used RAM (%)": {
        "type": "value",
        "units": "%",
        "value": "18"
    },
    "type": "container"
},
"SWAP info": {
    "SWAP usage": {
        "type": "value",
        "units": "MB",
        "value": "0"
    },
    "SWAP usage (%)": {
        "type": "value",
        "units": "%",
        "value": "0"
    },
    "Total SWAP": {
        "type": "value",
        "units": "MB",
        "value": "8191"
    },
    "type": "container"
}
```

# Summary
You've learned how to create your own module and can now expand the metrics collected by smu-server. [Next (creating static data modules)](2_static_data_modules.md)
