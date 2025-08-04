/**
 * @file metric_constructor.hpp
 * @headerfile core/internals/metric_constructor.hpp
 * @brief File with metric constructor
 *
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 * @license GPLv3 license, see LICENSE for details
 */

#pragma once

#include "metric_constructor_nodes.hpp"

namespace smu_server {




/**
 * @brief Creates a node with the value
 *
 * A value node is a node that only stores a value. It cannot store other nodes.
 *
 * @param metric_name Metric name
 * @param metric_value Metric value. Can be either a number or a string
 * @param value_units Metric units
 *
 * @tparam ValueType Value type. Must be convertible to `std::string`, i.e., `std::to_string` can be
 * called on it
 *
 * @return `std::unique_ptr` with `internals::MetricValueNode`
 *
 * @see `make_container_node` and `make_root_node`
 *
 * @section example_usage Example usage
 * @code{.cpp}
 * // The metric value can be either a number or a string, it is converted
 * // to a string if needed
 * auto node = make_value_node("RAM usage", 4217, "MB");
 * @endcode
 *
 * This code gives json fragment:
 *
 * @code{.json}
 * {
 *      "type": "value",
 *      "units": "MB",
 *      "value": "4217"
 * }
 * @endcode
 *
 * @note The node name is only used if the node is nested in a **container node** or **root node**.
 * **A value node never contains its own name!**
 */
template <typename ValueType>
inline auto make_value_node(const std::string& metric_name,
                            ValueType&&        metric_value,
                            const std::string& value_units)
    -> std::unique_ptr<internals::MetricValueNode> {

    std::string l_metric_value;

    // If ValueType is a number, convert it to a string
    if constexpr (std::is_arithmetic_v<ValueType>) {
        l_metric_value = std::to_string(metric_value);
    } else {
        l_metric_value = std::forward<ValueType>(metric_value);
    }

    return std::make_unique<internals::MetricValueNode>(
        metric_name, std::move(l_metric_value), value_units);
}




/**
 * @brief Creates a node that contains other nodes
 *
 * A container node can store other container nodes as well as values.
 * Such nodes are assumed to be nested either in other container nodes or in the
 * root node. If you need a **root** node, use `smu_server::make_root_node`
 *
 * @param container_name Name of container
 * @param children Nested nodes
 *
 * @tparam Children Types of nested nodes. Must inherit from `internals::IMetricNode`
 *
 * @return `std::unique_ptr` with `internals::MetricContainerNode<...>`
 *
 * @see `make_value_node` and `make_root_node`
 *
 * @section example_usage Example usage
 * @code{.cpp}
 * auto root = make_container_node("RAM",
 *      make_value_node("RAM usage", "4217", "MB")
 * );
 * @endcode
 *
 * This code gives json fragment:
 *
 * @code{.json}
 * {
 *      "RAM usage": {
 *          "type": "value",
 *          "units": "MB",
 *          "value": "4217"
 *      },
 *
 *      "type": "container"
 * }
 * @endcode
 *
 * @note The container node does not contain its name inside. The name is only used by higher-level
 * nodes.
 */
template <typename... Children>
inline auto make_container_node(const std::string& container_name, Children&&... children)
    -> std::unique_ptr<internals::MetricContainerNode<Children...>> {

    static_assert(sizeof...(children) > 0, "Container node must have at least one nested node");

    return std::make_unique<internals::MetricContainerNode<Children...>>(
        container_name, false, std::forward<Children>(children)...);
}




/**
 * @brief Creates a root node
 *
 * Root nodes work the same way as container nodes, but they doesn't store own name
 *
 * @param children Nested nodes, like **container** node or **value** node
 *
 * @tparam Children Types of nested nodes. Must inherit from `internals::IMetricNode`
 *
 * @return `std::unique_ptr` with `internals::MetricContainerNode<...>`
 *
 * @note Call `to_json` on the root node to get the json and, for example,
 * then send it over a websocket connection. However, this applies to all nodes, but it is used much
 * more often with **root** nodes.
 *
 * @see `make_value_node` and `make_container_node`
 *
 * @section example_usage Example usage
 * @code{.cpp}
 *
 * auto root = make_root_node(
 *      make_value_node("RAM usage", "4217", "MB")
 * );
 *
 * @endcode
 *
 * This code will produce a json of the form:
 *
 * @code{.json}
 * {
 *      "RAM usage": {
 *          "type": "value",
 *          "units": "MB",
 *          "value": "4217"
 *      },
 *
 *      "type": "container"
 * }
 *
 * @endcode
 *
 *
 * Consider another code:
 *
 * @code{.cpp}
 *
 * auto root = make_root_node(
 *      make_container_node("Simple container",
 *           // The metric value can be either a number or a string, it is
 *           // converted to a string if needed
 *           make_value_node("Simple value 1", 12345, "bytes"),
 *           make_value_node("Simple value 2", "42", "GB")
 *      ),
 *
 *      make_value_node("RAM usage", "4217", "MB")
 * );
 *
 * @endcode
 *
 *
 * And this code will produce a json of the form:
 *
 * @code{.json}
 *
 * {
 *		"RAM usage": {
 *	 		"type": "value",
 * 			"units": "MB",
 *			"value": "4217"
 *		},
 *
 *		"Simple container": {
 *			"Simple value 1": {
 *				"type": "value",
 *				"units": "bytes",
 *				"value": "12345"
 *			},
 *			"Simple value 2": {
 *				"type": "value",
 *				"units": "GB",
 *				"value": "42"
 *			},
 *
 *			"type": "container"
 *		},
 *
 *      "type": "container"
 * }
 *
 * @endcode
 */
template <typename... Children>
inline auto make_root_node(Children&&... children)
    -> std::unique_ptr<internals::MetricContainerNode<Children...>> {
    static_assert(sizeof...(children) > 0, "Root node must have at least one nested node");

    return std::make_unique<internals::MetricContainerNode<Children...>>(
        "", true, std::forward<Children>(children)...);
}




} // namespace smu_server
