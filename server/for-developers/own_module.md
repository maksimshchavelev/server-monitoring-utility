# Developing Your Own Module
This section will teach you how to develop your own module for smu-server.

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

When designing smu-server, we envisioned functionality that would maximize simplicity and standardization for module creation. We will explore this functionality next.

# IModule - Where It All Begins
All modules must inherit from the `IModule` class (otherwise it will cause a compilation error). This is a base abstract class that provides essential functionality. Its structure is shown below:

```cpp
class IModule {
  public:
    IModule(const Json::Value& configuration);
    virtual ~IModule() = default;

    virtual const Json::Value& get_configuration() const = 0;
    virtual std::optional<Json::Value> get_data() = 0;
    virtual void enable();
    virtual void disable();
    virtual bool is_enabled() const;

    virtual constexpr std::string_view module_name() const = 0;
    virtual constexpr std::string_view module_description() const = 0;

  protected:
    Json::Value m_configuration;
    bool        m_enabled{false};
};
```

Let's examine it.

### Constructor
Accepts `const Json::Value&` with the module configuration loaded from the corresponding file (see User Guide).

> ⚠️ If `configuration` is empty, the module must generate its own configuration and populate `m_configuration`. This configuration will be saved to a file and loaded on subsequent launches.

### `get_configuration`
Returns the module's current configuration. Used for saving configuration on application exit. Implement this method yourself.

### `get_data`
Forms a JSON with data and returns it. Returns `std::nullopt` in case of an error. We'll explore this in more detail in the practical section. Implement this method yourself.

### `enable` and `disable`
Enable and disable the module. You don't need to implement these methods (unless you have custom enable/disable logic). By default, they modify the `m_enabled` field.

### `is_enabled`
Returns whether the module is enabled or not. You don't need to implement this method (unless you have custom logic). By default, returns the value of the `m_enabled` field.

### `module_name` and `module_description`
Methods to get the module's name and description. **Never implement these yourself**, as they are properly implemented via the `REGISTER_MODULE` macro, which we'll discuss next.

## Practical Section
Let's create a RAM module that provides information about RAM and swap usage. Create two files: `include/modules/ram/ram.hpp` and `src/modules/ram/ram.cpp`.

Start with `ram.hpp`:
First, add the file description and copyright:

```cpp
/// GPLv3 LICENSE, Copyright (©) 2025, Your name <Your email>
/// See LICENSE for details

/**
 * @file modules/ram/ram.hpp
 * @brief File with RAM module class
 */
```

> ⚠️ The GPLv3 license is mandatory for all software components of smu!

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
    
    RAM(const Json::Value& config);
    const Json::Value& get_configuration() const override;
    std::optional<Json::Value> get_data() override;
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
/// GPLv3 LICENSE, Copyright (©) 2025, Your name <Your email>
/// See LICENSE for details

/**
 * @file modules/ram/ram.cpp
 * @brief File with RAM module class
 */

#include "modules/ram/ram.hpp"
#include <sys/sysinfo.h>
    
namespace smu_server {

// Public method
RAM::RAM(const Json::Value& config) : IModule(config) {
    // If config is empty
    if(m_configuration.empty()) {
        // Create new configuration
        m_configuration["enabled"] = true;
    }

    if(m_configuration["enabled"].asBool()) {
        // Module is disabled by default. See IModule
        enable();
    }
}


// Public method
const Json::Value& RAM::get_configuration() const {
    return m_configuration;
}


// Public method
std::optional<Json::Value> RAM::get_data() {
    if (!is_enabled())
        return std::nullopt;

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

} // namespace smu_server
```

Besides including `ram.hpp`, we include `<sys/sysinfo.h>` which contains the `sysinfo` system call we need.
In the constructor, we read the config and apply settings (here only the `enabled` setting controls module status). If we get an empty config, we configure from scratch. Note:

```cpp
if(m_configuration["enabled"].asBool()) {
    // Module is disabled by default. See IModule
    enable();
}
```
By default, the module is disabled (`m_enabled` is `false`, see [IModule](#imodule---where-it-all-begins)), so we enable it.

Next: The implementation of `get_configuration` is straightforward. Let's focus on `get_data`.

First, return `std::nullopt` if the module is disabled (also return `std::nullopt` on `sysinfo` error):

```cpp
if (!is_enabled())
     return std::nullopt;
```

Then we form the JSON with metric values. Let's examine this in detail.
The core idea is to group metrics (the smallest data unit in a module) using ***nodes***. A node can be:

- **Value Node**: Contains a value, unit of measurement, and metric name. Type: `value`
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
You've learned how to create your own module and can now expand the metrics collected by smu-server.
