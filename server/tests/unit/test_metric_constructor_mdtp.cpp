/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @brief Test for testing core/internals/metric_constructor.hpp (MDTP)
 */

#include "core/internals/metric_constructor.hpp"
#include <gtest/gtest.h>

using namespace smu_server;


// ---------- VALUE NODE ----------

TEST(MetricConstructor, ValueNode_MDTP_Layout) {
    // "load" = "56" "%"
    auto result = make_value_node("load", 56, "%")->to_mdtp();

    // ожидаемый общий размер: 1 + 4 + 4 + 4 + 1 + 4 + 2 = 20
    ASSERT_EQ(result.size(), 20u);

    // [0] node type
    EXPECT_EQ(result[0], 1);

    // [1..4] name length = 4
    EXPECT_EQ(internals::read_uint32_be(result, 1), 4u);

    // [5..8] name = "load"
    EXPECT_EQ(result[5], 'l');
    EXPECT_EQ(result[6], 'o');
    EXPECT_EQ(result[7], 'a');
    EXPECT_EQ(result[8], 'd');

    // [9..12] units length = 1
    EXPECT_EQ(internals::read_uint32_be(result, 9), 1u);

    // [13] units = "%"
    EXPECT_EQ(result[13], '%');

    // [14..17] value length = 2
    EXPECT_EQ(internals::read_uint32_be(result, 14), 2u);

    // [18..19] value = "56"
    EXPECT_EQ(result[18], '5');
    EXPECT_EQ(result[19], '6');
}


// ---------- CONTAINER NODE ----------

TEST(MetricConstructor, ContainerNode_WithTwoChildren_MDTP_Layout) {
    auto child1 = make_value_node("temperature", 75, "C")->to_mdtp();
    auto child2 = make_value_node("frequency",  "3.5", "GHz")->to_mdtp();

    ASSERT_EQ(child1.size(), 27u);
    ASSERT_EQ(child2.size(), 28u);

    auto container = make_container_node("cpu",
                                         make_value_node("temperature", 75, "C"),
                                         make_value_node("frequency",  "3.5", "GHz")
                                         )->to_mdtp();

    ASSERT_EQ(container.size(), 67u);

    // [0] node type
    EXPECT_EQ(container[0], 0);

    // [1..4] name length = 3
    EXPECT_EQ(internals::read_uint32_be(container, 1), 3u);

    // [5..7] name = "cpu"
    EXPECT_EQ(container[5], 'c');
    EXPECT_EQ(container[6], 'p');
    EXPECT_EQ(container[7], 'u');

    // [8..11] payload size = 55
    EXPECT_EQ(internals::read_uint32_be(container, 8), 55u);

    const size_t payload_offset = 12;

    // Child #1
    ASSERT_LE(payload_offset + child1.size(), container.size());
    EXPECT_TRUE(std::equal(child1.begin(), child1.end(), container.begin() + payload_offset));

    // Child #2
    const size_t child2_offset = payload_offset + child1.size();
    ASSERT_LE(child2_offset + child2.size(), container.size());
    EXPECT_TRUE(std::equal(child2.begin(), child2.end(), container.begin() + child2_offset));

    EXPECT_EQ(child2_offset + child2.size(), container.size());
}


// ---------- ROOT NODE ----------
TEST(MDTP_RootNode, FrameHeaderAndChildrenConcatenation) {
    // --- Arrange ---
    // Build children first so we can serialize them before moving into root.
    auto child1 = make_container_node(
        "RAM",
        make_value_node("Total", 8, "GB")
        );
    auto child2 = make_container_node(
        "CPU",
        make_value_node("Cores", 12, "cnt")
        );

    // Serialize children individually to form the expected payload.
    const auto bytes_child1 = child1->to_mdtp();
    const auto bytes_child2 = child2->to_mdtp();

    std::vector<uint8_t> expected_payload;
    expected_payload.insert(expected_payload.end(), bytes_child1.begin(), bytes_child1.end());
    expected_payload.insert(expected_payload.end(), bytes_child2.begin(), bytes_child2.end());

    // Now build the root frame (moves children).
    auto root = make_root_node(std::move(child1), std::move(child2));

    // --- Act ---
    const auto frame = root->to_mdtp();

    // --- Assert: frame header ---
    // [0] - MDTP version, [1..4] - payload size (big endian)
    ASSERT_GE(frame.size(), 5u) << "Root frame must contain header of 5 bytes";
    EXPECT_EQ(frame[0], static_cast<uint8_t>(MDTP_VERSION)) << "Invalid MDTP version in header";

    const uint32_t payload_size_be = internals::read_uint32_be(frame, 1);
    const uint32_t actual_payload_size = static_cast<uint32_t>(frame.size() - 5);
    EXPECT_EQ(payload_size_be, actual_payload_size)
        << "Header payload size must equal actual payload bytes";

    // --- Assert: payload equals concatenation of children ---
    std::vector<uint8_t> payload(frame.begin() + 5, frame.end());
    EXPECT_EQ(payload, expected_payload)
        << "Root payload must be a plain concatenation of children bytes";

    // --- Extra spot checks to ensure no extra containerization by root ---
    // Payload should start with the first child's node header (container => type=0).
    ASSERT_FALSE(payload.empty());
    EXPECT_EQ(payload[0], 0) << "First payload byte must be 'container' node type (0)";

    // Check the first child's name length and name ("RAM").
    ASSERT_GE(payload.size(), 1 + 4u);
    const uint32_t name_len_child1 = internals::read_uint32_be(payload, 1);
    EXPECT_EQ(name_len_child1, 3u) << "First container name length must be 3 ('RAM')";

    ASSERT_GE(payload.size(), 1 + 4u + name_len_child1);
    EXPECT_EQ(payload[5], 'R');
    EXPECT_EQ(payload[6], 'A');
    EXPECT_EQ(payload[7], 'M');

    // Sanity: the second child must follow immediately after the first child's bytes.
    ASSERT_GE(expected_payload.size(), bytes_child1.size() + 1u);
    ASSERT_GE(payload.size(), bytes_child1.size() + 1u);
    EXPECT_EQ(
        std::vector<uint8_t>(payload.begin() + bytes_child1.size(), payload.end()),
        bytes_child2
        ) << "Second child must follow the first child without extra bytes from root";
}


