# Creating modules with static data
The [previous chapter (first module)](1_own_module.md) described the creation of the RAM module. This module is constantly updating its data, but what if the data is static? After all, constantly creating a json tree ruins performance! Let's look at the solution in this chapter


## Intuition
It would be good if the server core polled the module once, saved the data, and did not poll it again. Fortunately, such a mechanism exists thanks to **poll_ratio**.


## What is poll ratio?
The poll ratio regulates the frequency of module polling. For example, if the poll ratio is 5, the module will be polled by the server core every 5 polling cycles. Thus, if the global polling interval is 1 second, the module will be polled every 5 seconds. This is necessary to avoid unnecessary polling if the module data is not updated that often.

But if you set the poll ratio to 0, the server core will cache the result of the first `get_data` call (if it is successful) and will send the cached data! In this case, the module will respond to `enable` and `disable` requests.


## Example
Let's consider the example of the `DMI` module constructor (module for obtaining static data from DMI/SMBIOS tables):

```cpp
DMI::DMI(const Config& configuration) : IModule(configuration) {
    // Create new configuration
    if (m_configuration.empty()) {
        m_configuration.set("enabled", true);

        // Cache data and prevent subsequent calls to `get_data`
        m_configuration.set("poll_ratio", 0);
    }

    // Check if the `enabled` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto enabled = m_configuration.get<bool>("enabled"); enabled.has_value()) {
        m_enabled = enabled.value();
    } else {
        log(LogType::ERROR, "The 'enabled' field is missing. Can't continue");
        throw std::runtime_error("The 'enabled' field is missing");
    }

    // Check if the `poll_ratio` field exists. If not, log the error and throw an exception to abort
    // registration.
    if (auto poll_ratio = m_configuration.get<unsigned int>("poll_ratio"); poll_ratio.has_value()) {
        m_poll_ratio = poll_ratio.value();
    } else {
        log(LogType::ERROR, "The 'poll_ratio' field is missing. Can't continue");
        throw std::runtime_error("The 'poll_ratio' field is missing");
    }

    // Collect information
    fill_bios_info();
    fill_board_info();
    fill_chassis_info();
    fill_product_info();
}
```

Please note that we set `poll_ratio` to zero by default. Thus, `get_data` will be called only once:

```cpp
std::optional<Json::Value> DMI::get_data() {
    auto root = make_root_node(
        // BIOS INFO
        make_container_node("BIOS",
                            make_value_node("BIOS date", m_bios_date, ""),
                            make_value_node("BIOS vendor", m_bios_vendor, ""),
                            make_value_node("BIOS version", m_bios_version, "")),

        // BOARD INFO
        make_container_node("Board",
                            make_value_node("Board name", m_board_name, ""),
                            make_value_node("Board vendor", m_board_vendor, ""),
                            make_value_node("Board version", m_board_version, "")),

        // CHASSIS INFO
        make_container_node("Chassis",
                            make_value_node("Chassis type", m_chassis_type, ""),
                            make_value_node("Chassis vendor", m_chassis_vendor, "")),

        // PRODUCT INFO
        make_container_node("Product",
                            make_value_node("Product name", m_product_name, ""),
                            make_value_node("Product family", m_product_family, ""),
                            make_value_node("Product serial", m_product_serial, ""),
                            make_value_node("Product UUID", m_product_uuid, "")));

    log(LogType::INFO, "Call");
    return root->to_json();
}
```

The JSON obtained from `root->to_json()` will be cached.

# Summary
In this chapter, two approaches to creating a module that stores static data were discussed. [Next chapter (storing json tree)](3_storing_json_tree.md)
