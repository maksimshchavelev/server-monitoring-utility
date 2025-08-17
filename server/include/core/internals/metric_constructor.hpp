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
 * @section example_usage Example usage (JSON)
 * @code{.cpp}
 * // The metric value can be either a number or a string; it is converted
 * // to a string if needed.
 * auto node = make_value_node("RAM usage", 4217, "MB")->to_json();
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
 * @section mdtp_protocol MDTP (preferred) — value node encoding
 *
 * Prefer the binary MDTP format for transport between modules and the server. Value nodes serialize
 * to the MDTP **value node** format (Big-Endian integer fields). The node produced by
 * `make_value_node("load", 56, "%")` serializes to the following bytes:
 *
 * @code{.text}
 * 01                   [node type] = 0x01 (value) (1)
 * 00 00 00 04          [name length] = 0x00000004 (4 bytes)
 * 6C 6F 61 64          [name] = "load"
 * 00 00 00 01          [units length] = 0x00000001 (1 byte)
 * 25                   [units] = "%"
 * 00 00 00 02          [value length] = 0x00000002 (2 bytes)
 * 35 36                [value] = "56"
 *                      total = 20 bytes
 * @endcode
 *
 * Fields are Big-Endian. For non-root nodes MDTP serialization returns the single node block shown
 * above (no global MDTP frame header). For full frames (root nodes) see `make_root_node` docs
 * below.
 *
 * @note Use `to_mdtp()` on the returned node to get the binary MDTP representation. The JSON API
 * (`to_json()`) is considered legacy for transport and is marked as deprecated here: prefer MDTP.
 *
 * @deprecated `to_json()` is deprecated for transport purposes; use `to_mdtp()` and MDTP for
 * serialization. `to_json()` may still be useful for debug/inspection, but it is not recommended
 * for network transport.
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
 * root node. If you need a **root** node, use `smu_server::make_root_node`.
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
 * @section example_usage Example usage (JSON)
 * @code{.cpp}
 * auto cpu = make_container_node("CPU",
 *      make_value_node("temperature", 75, "C"),
 *      make_value_node("frequency", "3.5", "GHz")
 * )->to_json();
 * @endcode
 *
 * This code gives json fragment:
 *
 * @code{.json}
 * {
 *      "temperature": {
 *          "type": "value",
 *          "units": "C",
 *          "value": "75"
 *      },
 *      "frequency": {
 *          "type": "value",
 *          "units": "GHz",
 *          "value": "3.5"
 *      },
 *      "type": "container"
 * }
 * @endcode
 *
 * @section mdtp_protocol MDTP (preferred) — container node encoding
 *
 * Container nodes serialize to MDTP as a single **container node** block which contains a payload
 * that is the concatenation of serialized child nodes. For example, the `cpu` container above
 * serializes as:
 *
 * Header (container):
 * @code{.text}
 * 00                 // [node type] = 0x00 (container) (1)
 * 00 00 00 03        // [name length] = 0x00000003 (3 bytes) -> "CPU"
 * 63 70 75           // [name] = "CPU"
 * 00 00 00 37        // [payload size] = 0x00000037 (55 bytes)
 *                    // (payload follows: child1 (27 bytes) + child2 (28 bytes))
 * @endcode
 *
 * Child 1 (temperature, 27 bytes):
 * @code{.text}
 * 01                                 // type = value
 * 00 00 00 0B                        // name_len = 11 ("temperature")
 * 74 65 6D 70 65 72 61 74 75 72 65   // "temperature"
 * 00 00 00 01                        // units_len = 1
 * 43                                 // "C"
 * 00 00 00 02                        // value_len = 2
 * 37 35                              // "75"
 * @endcode
 *
 * Child 2 (frequency, 28 bytes):
 * @code{.text}
 * 01
 * 00 00 00 09                         // name_len = 9 ("frequency")
 * 66 72 65 71 75 65 6E 63 79          // "frequency"
 * 00 00 00 03                         // units_len = 3 ("GHz")
 * 47 48 5A                            // "GHz"
 * 00 00 00 03                         // value_len = 3 ("3.5")
 * 33 2E 35                            // "3.5"
 * @endcode
 *
 * Combined container total size = container header (1 + 4 + 3 + 4 = 12 ? careful: see note)
 *
 * @note The container header consists of:
 *  - 1 byte: node type
 *  - 4 bytes: name length (N)
 *  - N bytes: name
 *  - 4 bytes: payload size (sum of child blocks)
 *
 * The `payload size` above is 55 (0x37) because 27 + 28 = 55.
 *
 * @note For non-root container nodes, MDTP serialization returns exactly the bytes above
 * (container header + concatenated children). For root nodes, a global MDTP frame header is added
 * (see `make_root_node` documentation).
 *
 * @deprecated `to_json()` is deprecated for transport — prefer `to_mdtp()` / MDTP for efficient
 * binary transport.
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
 * Root nodes work the same way as container nodes, but they do not store their own name.
 *
 * @param children Nested nodes, like **container** node or **value** node
 *
 * @tparam Children Types of nested nodes. Must inherit from `internals::IMetricNode`
 *
 * @return `std::unique_ptr` with `internals::MetricContainerNode<...>`
 *
 * @see `make_value_node` and `make_container_node`
 *
 * @section example_usage Example usage (JSON)
 * @code{.cpp}
 * auto root = make_root_node(
 *      make_value_node("RAM usage", "4217", "MB")
 * )->to_json();
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
 *      "type": "container"
 * }
 * @endcode
 *
 * @section mdtp_protocol MDTP (preferred) — full MDTP frame emitted by root
 *
 * Root nodes produce a **full MDTP frame**: a 5-byte frame header followed by the usual container
 * node encoding (container header + concatenated children). Frame header fields:
 *
 * - 1 byte: MDTP version (currently `0x01`)
 * - 4 bytes: frame payload size (Big-Endian uint32) — the number of bytes after the frame header,
 *   i.e. `container header + container payload`
 *
 * Example: two children:
 *  - `a = make_value_node("uptime", "1234", "s")`  -> 24 bytes
 *  - `b = make_value_node("load", 56, "%")`        -> 20 bytes
 *
 * children concat size = 24 + 20 = 44 bytes (0x2C)
 *
 * Container header (root container with empty name):
 * @code{.text}
 * 00                                    // [node type] = 0x00 (container)
 * 00 00 00 00                           // [name length] = 0x00000000 (0 bytes) — root has no name
 * 00 00 00 2C                           // [container payload size] = 0x0000002C (44 bytes)
 *                                       // container header total = 1 + 4 + 0 + 4 = 9 bytes
 * @endcode
 *
 * Frame header (prefixing the container):
 * @code{.text}
 * 01                                    // [MDTP version] = 0x01 (1)
 * 00 00 00 35                           // [frame payload size] = 0x00000035 (53 bytes)
 *                                       // 53 = container header (9) + children (44)
 *                                       // frame header total = 1 + 4 = 5 bytes
 * @endcode
 *
 * Full frame layout (frame header + container header + children) — total = 5 + 9 + 44 = 58 bytes.
 *
 * @note Use `to_mdtp()` on the root node to obtain the full MDTP frame ready to be written to the
 * network or stored. MDTP is Big-Endian; your sender/receiver must honor endianness.
 *
 * @deprecated `to_json()` is deprecated for transport. `to_mdtp()`/MDTP should be used for
 * binary transport between modules and the server core. `to_json()` may remain useful for human
 * readable debugging, but it is not recommended for production transport.
 *
 * @section examples_more Advanced usage notes
 *
 * - If you need human readable output (debug) use `to_json()` for inspection only.
 * - If you need compact, fast transport (recommended) use `to_mdtp()` on the root and send the
 *   returned `std::vector<uint8_t>` over your socket. The first byte is MDTP version which helps
 *   future protocol negotiation.
 *
 * @note All length fields in MDTP are counts of bytes **without** a terminating NUL. Strings are
 * transmitted as raw bytes; they are NOT NUL-terminated in the stream.
 */

template <typename... Children>
inline auto make_root_node(Children&&... children)
    -> std::unique_ptr<internals::MetricContainerNode<Children...>> {
    static_assert(sizeof...(children) > 0, "Root node must have at least one nested node");

    return std::make_unique<internals::MetricContainerNode<Children...>>(
        "", true, std::forward<Children>(children)...);
}




} // namespace smu_server
