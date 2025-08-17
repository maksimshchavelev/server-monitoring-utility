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
TEST(MetricConstructor, RootNode_WithFrameHeader) {
    // Prepare children
    auto a = make_value_node("uptime", "1234", "s")->to_mdtp(); // 24 bytes
    auto b = make_value_node("load", 56, "%")->to_mdtp();       // 20 bytes
    ASSERT_EQ(a.size(), 24u);
    ASSERT_EQ(b.size(), 20u);

    std::vector<uint8_t> children_concat;
    children_concat.insert(children_concat.end(), a.begin(), a.end());
    children_concat.insert(children_concat.end(), b.begin(), b.end());
    ASSERT_EQ(children_concat.size(), 44u);

    auto root = make_root_node(
                    make_value_node("uptime", "1234", "s"),
                    make_value_node("load", 56, "%")
                    )->to_mdtp();

    // Sizes
    const size_t frame_header = 5;          // version + payload size
    const size_t container_header = 9;      // type + name_len + name(0) + payload_size
    const size_t children_size = 44;
    const size_t expected_total = frame_header + container_header + children_size; // 58

    ASSERT_EQ(root.size(), expected_total);

    // --- Frame header ---
    EXPECT_EQ(root[0], 1); // version

    // Payload size should include container header + children = 9 + 44 = 53
    EXPECT_EQ(internals::read_uint32_be(root, 1), 53u);

    // --- Root container header (starts at offset 5) ---
    const size_t offset = frame_header;

    EXPECT_EQ(root[offset], 0); // type = container
    EXPECT_EQ(internals::read_uint32_be(root, offset + 1), 0u); // name length = 0
    EXPECT_EQ(internals::read_uint32_be(root, offset + 5), 44u); // payload size = children

    // --- Root container payload (children) ---
    EXPECT_TRUE(std::equal(children_concat.begin(), children_concat.end(),
                           root.begin() + frame_header + container_header));
}


