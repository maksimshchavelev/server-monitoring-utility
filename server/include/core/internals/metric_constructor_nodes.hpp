/**
 * @file metric_constructor_nodes.hpp
 * @brief File with metric constructor
 *
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 * @license GPLv3 license, see LICENSE for details
 */

#pragma once

#include <json/json.h>
#include <tuple>
#include <utils/for_each_tuple.hpp>

namespace smu_server {

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
 */
template <typename... Children> class IMetricNode : public IMetricNodeBase {
  public:
    /**
     * @brief IMetricNode constructor
     * @param node_name Name of node. Used by the parent container. Not used if the node is the root
     * node
     * @param value Value of node. Used if node is a value
     * @param units The unit of measurement of the node value. Used if the node is a value
     * @param is_root_container Must be true if the node is a root container
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
     * @brief Get name of node
     * @return Node name
     */
    virtual const std::string& get_name() const noexcept {
        return m_name;
    }


  protected:
    std::string                                   m_name;     ///< node name
    std::string                                   m_value;    ///< node value (if node type is value)
    std::string                                   m_units;    ///< node units (if node type is value)
    bool                                          m_is_root;  ///< `true` if node type is root
    [[no_unique_address]] std::tuple<Children...> m_children; ///< children (if node type is **conatiner** or **root**)
};




// ========================================================================
// =========================== MetricValueNode ===========================
// ========================================================================



/**
 * @brief Implements a value node
 */
class MetricValueNode : public IMetricNode<> {
  public:
    /**
     * @brief MetricValueNode constructor
     * @param node_name Name of node.
     * @param node_value Value of node
     * @param value_units Units of value
     */
    MetricValueNode(const std::string& node_name,
                    const std::string& node_value,
                    const std::string& value_units) :
        IMetricNode(node_name, node_value, value_units, false) {}




    /**
     * @brief Get json
     * @return The generated json
     */
    Json::Value to_json() const override {
        Json::Value root;

        root["type"] = "value";
        root["value"] = m_value;
        root["units"] = m_units;

        return root;
    }
};




// ======================================================================
// =========================== MetricContainerNode =====================
// ======================================================================




/**
 * @brief Implements container with nodes
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
     */
    MetricContainerNode(const std::string& node_name,
                        bool               is_root_container,
                        Children&&... children) :
        IMetricNode<Children...>(
            node_name, "", "", is_root_container, std::forward<Children>(children)...) {}




    /**
     * @brief Get json
     * @return The generated json
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
};

} // namespace internals

} // namespace smu_server
