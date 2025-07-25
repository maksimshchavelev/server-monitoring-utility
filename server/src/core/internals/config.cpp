/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/config.cpp
 * @brief Thread-safe wrapper for storing Json configs
 */

#include "core/internals/config.hpp"
#include <mutex>

namespace smu_server {


// Public constructor
Config::Config() : m_json(Json::ValueType::objectValue) {}




// Public constructor
Config::Config(Json::Value json) : m_json(json) {}




// Public constructor
Config::Config(const Config& other) {
    // Safe locking without deadlock risk
    std::scoped_lock lock(m_json_mutex, other.m_json_mutex);

    m_json = other.m_json;
}




// Public constructor
Config::Config(Config&& other) {
    // Safe locking without deadlock risk
    std::scoped_lock lock(m_json_mutex, other.m_json_mutex);

    m_json = std::move(other.m_json);
}




// Public operator
Config& Config::operator=(const Config& other) {
    // Prevent deadlock when we assign ourselves
    if (this == &other) {
        return *this;
    }

    // Safe locking without deadlock risk
    std::scoped_lock lock(m_json_mutex, other.m_json_mutex);

    m_json = other.m_json;
    return *this;
}




// Public operator
Config& Config::operator=(Config&& other) {
    // Prevent deadlock when we assign ourselves
    if (this == &other) {
        return *this;
    }

    // Safe locking without deadlock risk
    std::scoped_lock lock(m_json_mutex, other.m_json_mutex);

    m_json = std::move(other.m_json);
    return *this;
}




// Public operator
bool Config::operator==(const Config& other) const {
    // Prevent deadlock when we compare ourselves
    if (this == &other) {
        return true;
    }

    // Safe locking without deadlock risk
    std::scoped_lock lock(m_json_mutex, other.m_json_mutex);

    return m_json == other.m_json;
}




// Public method
std::optional<Config> Config::get_subconfig(const std::string& subconfig_name) const {
    return get<Config>(subconfig_name);
}




// Public method
std::size_t Config::size() const {
    std::lock_guard<std::mutex> lock(m_json_mutex);
    return m_json.size();
}




// Public method
template <> std::optional<Config> Config::get<Config>(const std::string& key) const {
    std::lock_guard<std::mutex> lock(m_json_mutex);

    // If found
    if (m_json.isMember(key) && m_json[key].isObject()) {
        try {
            return Config(m_json[key]); // as can throw exception
        } catch (const std::exception& e) {
            return std::nullopt;
        }
    }

    // If not found
    return std::nullopt;
}


} // namespace smu_server
