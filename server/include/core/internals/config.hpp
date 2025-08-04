/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/config.hpp
 * @brief Thread-safe wrapper for storing Json configs
 */

#pragma once

#include <functional>
#include <json/json.h>

namespace smu_server {

/**
 * @brief Thread-safe wrapper for storing Json configs
 */
class Config {
  public:
    /**
     * @brief Default constructor
     */
    Config();




    /**
     * @brief Constructor from `Json::Value`
     * @param json Json
     */
    Config(Json::Value json);




    /**
     * @brief Copy constructor
     * @param other Other `Config` object
     */
    Config(const Config& other);




    /**
     * @brief Move constructor
     *
     * Moves another `Config`. `other` becomes empty.
     *
     * @param other Other `Config` object
     */
    Config(Config&& other);




    /**
     * @brief Copy operator =
     *
     * Just copies another `Config`
     *
     * @param other Other `Config` object
     *
     * @return `Config&` (self)
     */
    Config& operator=(const Config& other);




    /**
     * @brief Move operator =
     *
     * Moves another `Config`. `other` becomes empty.
     *
     * @param other Other `Config` object
     *
     * @return `Config&` (self)
     */
    Config& operator=(Config&& other);




    /**
     * @brief operator ==
     * @param other Other `Config` object
     * @return `true` if objects are equal, otherwise `false`
     */
    bool operator==(const Config& other) const;




    /**
     * @brief Set the value associated with the key. If there is no associated value, a new one will
     * be created.
     *
     * @param key Key
     * @param value Value
     *
     * @note Empty `key` will be ignored
     *
     * @section example_usage Example usage
     * @code{.cpp}
     * Config config;
     * config.set("key", 10);
     * @endcode
     */
    template <typename T>
    void set(const std::string& key, T&& value)
        requires requires {
            Json::Value()[std::string()] = std::forward<T>(value);
        } || std::is_same_v<Config, std::decay_t<T>>
    {
        if (key.empty()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_json_mutex);

        if constexpr (std::is_same_v<Config, std::decay_t<T>>) {
            m_json[key] = std::forward_like<T>(value.m_json);
        } else {
            m_json[key] = std::forward<T>(value);
        }
    }




    /**
     * @brief Returns the value by key
     *
     * @param key Key
     *
     * @return The value, if it exists, otherwise `std::nullopt` (or if there was an attempt to
     * obtain an inappropriate type)
     *
     * @warning Always call `has_value()` on the returned value before using it, because this method
     * may not return a value.
     *
     * @section example_usage Example usage
     * For example, `config` has already been created and has the following structure:
     * @code{.json}
     * {
     *      "key": 42
     * }
     * @endcode
     *
     * Then, when executing this code:
     * @code{.cpp}
     * if (auto value = config.get<int>("key"); value.has_value()) {
     *      std::cout << value.value();
     * } else {
     *      std::cout << "Error";
     * }
     * @endcode
     *
     * The output should be as follows:
     *
     * ```
     * 42
     * ```
     */
    template <typename DesiredType> std::optional<DesiredType> get(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_json_mutex);

        // If found
        if (m_json.isMember(key)) {
            try {
                return m_json[key].as<DesiredType>(); // `as` can throw exception
            } catch (const std::exception& e) {
                return std::nullopt;
            }
        }

        // If not found
        return std::nullopt;
    }




    /**
     * @brief Provides **more efficient** thread-safely access to the field by const reference
     *
     * @param key Key
     * @param callback The function to which the value will be passed
     *
     * @note If the `function` throws an exception, there will be no effect.
     *
     * If the field is missing or an error occurred while receiving it, `std::nullopt` will be
     * passed to the function.
     *
     * @warning Always call `has_value()` on the returned value before using it, because this method
     * may not return a value.
     *
     * @section example_usage Example usage
     * For example, `config` has already been created and has the following structure:
     * @code{.json}
     * {
     *      "key": 42
     * }
     * @endcode
     *
     * Then, when executing this code:
     *
     * @code{.cpp}
     * config.get<int>("key", [](const std::optional<int>& value){
     *      if (value.has_value()) {
     *          std::cout << value.value() << std::endl;
     *      } else {
     *          std::cout << "Error" << std::endl;
     *      }
     * });
     * @endcode
     *
     * The output should be as follows:
     *
     * ```
     * 42
     * ```
     */
    template <typename DesiredType>
    void get(const std::string&                                           key,
             std::function<void(const std::optional<DesiredType>& value)> callback) const {
        std::lock_guard<std::mutex> lock(m_json_mutex);

        if (m_json.isMember(key)) {
            try {
                const auto& value = m_json[key].as<DesiredType>();
                try {
                    callback(value);
                } catch (...) { // do nothing
                }
            } catch (...) { // receiving error
                try {
                    callback(std::nullopt);
                } catch (...) { // do nothing
                }
            }
        } else {
            try {
                callback(std::nullopt);
            } catch (...) { // do nothing
            }
        }
    }




    /**
     * @brief Allows you to obtain raw Json::Value thread-safely
     *
     * @param callback The function to which the `Json::Value` will be passed
     *
     * @note If the `function` throws an exception, there will be no effect.
     *
     * @section example_usage Example usage
     * For example, `config` has already been created and has the following structure
     * @code{.json}
     * {
     *      "key1": "value",
     *      "key2: 10
     * }
     * @endcode
     *
     * Then, when executing this code:
     *
     * @code{.cpp}
     * config.get([&](const Json::Value& value){
     *      std::cout << value["key1"].as<std::string>() << std::endl;
     *      std::cout << value["key2"].as<int>() << std::endl;
     * });
     * @endcode
     *
     * The output should be as follows:
     *
     * ```
     * value
     * 10
     * ```
     */
    void get(std::function<void(const Json::Value& json)> callback) const {
        std::lock_guard<std::mutex> lock(m_json_mutex);

        try {
            callback(m_json);
        } catch (...) { // do nothing
        }
    }




    /**
     * @brief Get subconfig. Similar to calling `get<Config>("subconfig name")`
     *
     * @param subconfig_name Subconfig name
     *
     * @return `std::optional` with `Config`, if it exists, otherwise `std::nullopt`
     *
     * @warning Always call `has_value()` on the returned value before using it, because this method
     * may not return a value.
     *
     * @see get
     */
    std::optional<Config> get_subconfig(const std::string& subconfig_name) const;




    /**
     * @brief Allows you to obtain a **copy** of the internal `Json::Value`
     *
     * @return `Json::Value`
     *
     * @note Use `Config::get` **for more efficient use**
     *
     * @see get
     */
    Json::Value get_json() const;




    /**
     * @brief Get config size
     * @return `std::size` with size
     */
    std::size_t size() const;




    /**
     * @brief Checks if `Config` is empty
     * @return `true` if empty, otherwise `false`
     */
    bool empty() const;


  private:
    Json::Value        m_json;       ///< Internal json object
    mutable std::mutex m_json_mutex; ///< Mutex to prevent data race with `m_json`
};


/**
 * @brief Specialization of `Config::get` for template parameter `Config` to get subconfigs
 * @param key Key
 * @return The value, if it exists and value is `Json::Value` object, otherwise `std::nullopt`
 * (or if there was an attempt to obtain an inappropriate type)
 *
 * @warning Always call `has_value()` on the returned value before using it, because this method may
 * not return a value.
 *
 * @section example_usage Example usage
 * For example, config has this structure:
 *
 * @code{.json}
 * {
 *      "key 1": 5,
 *      "subconfig": {
 *          "key 2": "value 2",
 *          "key 3": "value 3"
 *      }
 * }
 * @endcode
 *
 * And the code below is executed (`config` is already created):
 *
 * @code{.cpp}
 * auto subconfig = config.get<Config>("subconfig");
 * @endcode
 *
 * Then `subconfig` will have the following structure:
 *
 * @code{.json}
 * {
 *      "key 2": "value 2",
 *      "key 3": "value 3"
 * }
 * @endcode
 */
template <> std::optional<Config> Config::get<Config>(const std::string& key) const;


} // namespace smu_server
