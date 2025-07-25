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
     * @param other Other `Config` object
     */
    Config(Config&& other);




    /**
     * @brief Copy operator =
     * @param other Other `Config` object
     * @return `Config&` (self)
     */
    Config& operator=(const Config& other);




    /**
     * @brief Move operator =
     * @param other Other `Config` object
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
     * @param key Key
     * @param value Value
     * @note Empty key will be ignored
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
     * @param key Key
     * @return The value, if it exists, otherwise `std::nullopt` (or if there was an attempt to
     * obtain an inappropriate type)
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
     * @brief Provides more efficient thread-safely access to the field by const reference
     * @param key Key
     * @param callback The function to which the value will be passed
     * @note If the function throws an exception, there will be no effect.
     * If the field is missing or an error occurred while receiving it, `std::nullopt` will be
     * passed to the function.
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
     * @param callback The function to which the `Json::Value` will be passed
     * @note If the function throws an exception, there will be no effect.
     */
    void get(std::function<void(const Json::Value& json)> callback) const {
        std::lock_guard<std::mutex> lock(m_json_mutex);

        try {
            callback(m_json);
        } catch (...) { // do nothing
        }
    }




    /**
     * @brief Get subconfig. Similar to calling `get<Json::Value>(“subconfig name”)`
     * @param subconfig_name Subconfig name
     * @return `std::optional<Config>`
     * @see get
     */
    std::optional<Config> get_subconfig(const std::string& subconfig_name) const;




    /**
     * @brief Allows you to obtain a copy of the internal `Json::Value`
     * @return `Json::Value`
     * @see get for more *efficient* use
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
    Json::Value        m_json;
    mutable std::mutex m_json_mutex;
};


/**
 * @brief Specialization `get` for template parameter `Config`
 * @param key Key
 * @return The value, if it exists and value is `Json::Value` object, otherwise `std::nullopt`
 * (or if there was an attempt to obtain an inappropriate type)
 */
template <> std::optional<Config> Config::get<Config>(const std::string& key) const;


} // namespace smu_server
