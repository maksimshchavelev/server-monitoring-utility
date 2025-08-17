/**
 * @file metric_constructor_nodes.hpp
 * @brief File with metric constructor
 *
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 * @license GPLv3 license, see LICENSE for details
 */

#pragma once

#include "memutils.hpp"
#include <json/json.h>
#include <tuple>
#include <utils/for_each_tuple.hpp>

namespace smu_server {

constexpr uint8_t MDTP_VERSION = 1;

/**
 * @brief The Base IMetricNode class to simplify the storage of data obtained from
 * `smu_server::make_root_node`
 *
 * First, you can create a smart pointer to `IMetricNodeBase` if you create nodes
 * via factory methods in the module constructor, in which case you can get json by calling
 * IMetricNodeBase::to_json().
 *
 * @section example_usage Example usage
 *
 * @code{.cpp}
 *
 * std::unique_ptr<IMetricNodeBase> m_root = make_root_node( ... );
 * Json::Value m_json = m_root->to_json();
 *
 * @endcode
 *
 * @see smu_server::internals::IMetricNode (**don't use directly!**)
 * @headerfile core/core.hpp
 */
struct IMetricNodeBase {
    /**
     * @brief Recursively generate json
     *
     * Recursively creates `Json::Value` and returns it
     *
     * @warning Do not call this function too often, as it may cause performance degradation
     *
     * @return The generated json
     */
    virtual Json::Value to_json() const = 0;


    /**
     * @brief Generates binary data in MDTP protocol format.
     *
     * See the developer documentation for the MDTP protocol specification and examples.
     *
     * @return `std::vector<uint8_t>` with bytes
     */
    virtual std::vector<uint8_t> to_mdtp() const = 0;


    /**
     * @brief Does nothing
     */
    virtual ~IMetricNodeBase() = default;
};


namespace internals {


// ===================================================================
// =========================== IMetricNode ===========================
// ===================================================================



/**
 * @brief Interface IMetricNode class
 *
 * Is an interface (abstract) node class. Can be a node of any type
 *
 * @tparam Children Child nodes. Must inherit from IMetricNode.
 *
 * @note This is an internal class for creating nodes. Use the factory methods
 * `smu_server::make_root_node`, `smu_server::make_container_node`, `smu_server::make_value_node`
 * for convenient and error-free node creation.
 */
template <typename... Children> class IMetricNode : public IMetricNodeBase {
  public:
    /**
     * @brief IMetricNode constructor
     * @param node_name Name of node. Used by the parent container. **Not used if the node type is
     * the `root` node**
     * @param value Value of node. Used if **node type** is a `value`
     * @param units The unit of measurement of the **value node**. Used if the **node type** is a
     * `value`
     * @param is_root_container Must be `true` if the node is a **root** container. **Root nodes do
     * not contain a name**
     * @param children Nested nodes
     *
     * @note `children` must be an inheritor of IMetricNode
     */
    IMetricNode(const std::string& node_name,
                const std::string& value,
                const std::string& units,
                bool               is_root_container,
                Children&&... children) noexcept
        requires(std::is_base_of_v<IMetricNodeBase,
                                   typename std::remove_cvref_t<Children>::element_type> &&
                 ...)
        :
        m_name(node_name), m_value(value), m_units(units), m_is_root(is_root_container),
        m_children(std::forward<Children>(children)...) {}




    /**
     * @brief Default virtual constructor. Doesn't do anything
     */
    virtual ~IMetricNode() = default;




    /**
     * @brief Recursively generates json, preserving the hierarchy
     * of nodes, their types, values and names
     * @return The generated json
     */
    virtual Json::Value to_json() const = 0;




    /**
     * @brief Generates binary data in MDTP protocol format.
     *
     * See the developer documentation for the MDTP protocol specification and examples.
     *
     * @return `std::vector<uint8_t>` with bytes
     */
    virtual std::vector<uint8_t> to_mdtp() const = 0;




    /**
     * @brief Get name of node
     * @return Node name
     */
    virtual const std::string& get_name() const noexcept {
        return m_name;
    }


  protected:
    std::string m_name;    ///< node name
    std::string m_value;   ///< node value (if node type is value)
    std::string m_units;   ///< node units (if node type is value)
    bool        m_is_root; ///< `true` if node type is root
    [[no_unique_address]] std::tuple<Children...>
        m_children; ///< children (if node type is **conatiner** or **root**)
};




// ========================================================================
// =========================== MetricValueNode ===========================
// ========================================================================



/**
 * @brief Implements a value node
 *
 * A value node can only store a value. Such a node does not contain any nested nodes.
 *
 * @note This is an internal class for creating nodes. Use the factory methods
 * `smu_server::make_root_node`, `smu_server::make_container_node`, `smu_server::make_value_node`
 * for convenient and error-free node creation.
 */
class MetricValueNode : public IMetricNode<> {
  public:
    /**
     * @brief MetricValueNode constructor
     * @param node_name Name of node.
     * @param node_value Value of node
     * @param value_units Units of value
     *
     * @section example Example
     * For example, if we pass:
     * - `node_name` = "RAM usage"
     * - `node_value` = "5740"
     * - `value_units` = "MB"
     *
     * Then calling the `to_json` method will return the following JSON:
     * ```{.json}
     * {
     *      "type": "value",
     *      "value": "5740",
     *      "units": "MB"
     * }
     *
     * ```
     *
     * **Please note that the value node does not store its name. To get the name, call
     * `get_name`**
     *
     * @see get_name
     * @see to_json
     *
     */
    MetricValueNode(const std::string& node_name,
                    const std::string& node_value,
                    const std::string& value_units) :
        IMetricNode(node_name, node_value, value_units, false) {}




    /**
     * @brief Recursively generates json, preserving the hierarchy
     * of nodes, their types, values and names
     * @return The generated json
     */
    Json::Value to_json() const override {
        Json::Value root;

        root["type"] = "value";
        root["value"] = m_value;
        root["units"] = m_units;

        return root;
    }




    /**
     * @brief Generates binary data in MDTP protocol format.
     *
     * See the developer documentation for the MDTP protocol specification and examples.
     *
     * @return `std::vector<uint8_t>` with bytes
     */
    std::vector<uint8_t> to_mdtp() const override {
        std::vector<uint8_t> result;
        result.resize(1 /* node type */ + 4 /* name length */ + m_name.length() /* name */ +
                          4 /* units length */ + m_units.length() /* units */ +
                          4 /* value length */ + m_value.length() /* value */,
                      0x0 /* fill by 0x0 */);
        std::size_t offset = 0;

        // Write node type (1 is value node)
        write_ubyte_be(result, offset, 1);
        ++offset;

        // Write name length
        write_uint32_be(result, offset, static_cast<uint32_t>(m_name.length()));
        offset += 4;

        // Write name
        std::copy(m_name.begin(), m_name.end(), result.data() + offset);
        offset += m_name.length();

        // Write units length
        write_uint32_be(result, offset, static_cast<uint32_t>(m_units.length()));
        offset += 4;

        // Write units
        std::copy(m_units.begin(), m_units.end(), result.data() + offset);
        offset += m_units.length();

        // Write value length
        write_uint32_be(result, offset, static_cast<uint32_t>(m_value.length()));
        offset += 4;

        // Write value
        std::copy(m_value.begin(), m_value.end(), result.data() + offset);

        return result;
    }
};




// ======================================================================
// =========================== MetricContainerNode =====================
// ======================================================================




/**
 * @brief Implements container with nodes
 *
 * It can be either a container node or a root node.
 *
 * @note This is an internal class for creating nodes. Use the factory methods
 * `smu_server::make_root_node`, `smu_server::make_container_node`, `smu_server::make_value_node`
 * for convenient and error-free node creation.
 */
template <typename... Children> class MetricContainerNode : public IMetricNode<Children...> {
  public:
    /**
     * @brief MetricContainerNode constructor
     * @param node_name Name of node. Not used if the node is root node
     * @param is_root_container Is the node a root node?
     * @param children Nested nodes
     *
     * @note `children` must be an inheritor of IMetricNode
     *
     * @see get_name
     * @see to_json
     */
    MetricContainerNode(const std::string& node_name,
                        bool               is_root_container,
                        Children&&... children) :
        IMetricNode<Children...>(
            node_name, "", "", is_root_container, std::forward<Children>(children)...) {}




    /**
     * @brief Recursively generates json, preserving the hierarchy
     * of nodes, their types, values and names
     * @return The generated json
     *
     * @note The name of nested nodes is added automatically. For example, the returned JSON may
     * look like this:
     * ```{.json}
     * {
     *      "RAM usage": {
     *          "type": "value",
     *          "value": "5740",
     *          "units": "MB"
     *      }
     * }
     * ```
     */
    Json::Value to_json() const override {
        Json::Value root;

        // We don't need to store the type if we are root node
        if (!IMetricNode<Children...>::m_is_root) {
            root["type"] = "container";
        }

        // Iterate through the descendants and recursively call get_json. The recursion will stop
        // as soon as we reach the node-value. The obtained objects are placed with the desired
        // name in root and return
        for_each_tuple(IMetricNode<Children...>::m_children,
                       [this, &root](auto& child) { root[child->get_name()] = child->to_json(); });

        return root;
    }




    /**
     * @brief Generates binary data in MDTP protocol format.
     *
     * See the developer documentation for the MDTP protocol specification and examples.
     *
     * @return `std::vector<uint8_t>` with bytes
     */
    std::vector<uint8_t> to_mdtp() const override {
        std::vector<uint8_t> result;
        result.resize(1 /* node type */ + 4 /* name length */ +
                          IMetricNode<Children...>::m_name.length() /* name */
                          + 4 /* payload size */ + 0 /* payload */,
                      0x0 /* fill by 0x0 */);

        std::size_t offset = 0;

        // If node type is root
        if (IMetricNode<Children...>::m_is_root) {
            // Add header
            result.resize(result.size() + 1 /* version */ + 4 /* payload size */);
            offset = 5; // After header
        }

        // Payload
        std::vector<uint8_t> payload;

        // Iterate through the descendants and recursively call to_mdtp. The recursion will stop
        // as soon as we reach the node-value. The obtained objects are placed with the desired
        // name in root and return
        for_each_tuple(IMetricNode<Children...>::m_children, [this, &payload](auto& child) {
            auto child_data = child->to_mdtp();
            payload.insert(payload.end(), child_data.begin(), child_data.end());
        });

        // Write node type
        write_ubyte_be(result, offset, 0);
        ++offset;

        // Write node name length
        write_uint32_be(
            result, offset, static_cast<uint32_t>(IMetricNode<Children...>::m_name.length()));
        offset += 4;

        // Write node name
        std::copy(IMetricNode<Children...>::m_name.begin(),
                  IMetricNode<Children...>::m_name.end(),
                  result.data() + offset);
        offset += IMetricNode<Children...>::m_name.length();

        // Write payload size
        write_uint32_be(result, offset, static_cast<uint32_t>(payload.size()));
        offset += 4;

        // Write payload
        std::copy(payload.begin(), payload.end(), std::back_inserter(result));

        // If node type is root, insert header
        if (IMetricNode<Children...>::m_is_root) {
            write_ubyte_be(result, 0, MDTP_VERSION);                           // write version
            write_uint32_be(result, 1, static_cast<uint32_t>(payload.size())); // write payload size
        }

        return result;
    }
};

} // namespace internals

} // namespace smu_server
