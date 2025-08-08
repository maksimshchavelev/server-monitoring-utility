/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @brief Test for testing core/internals/config.hpp
 */

#include "core/internals/config.hpp"
#include <gtest/gtest.h>

using namespace smu_server;

// =============================== CONSTRUCTORS ===============================

TEST(DefaultConstructorTest, ConstructEmpty) {
    Config config;
    ASSERT_EQ(config.size(), 0);
}


TEST(CostructorFromJson, EmptyJson) {
    Json::Value json;
    Config      config(json);
    ASSERT_EQ(config.size(), 0);
}


TEST(ConstructorFromJson, OneValueJson) {
    Json::Value json;
    json["key"] = 0;


    Config config(json);
    ASSERT_EQ(config.size(), 1);
}


TEST(ConstructorFromConfig, EmptyOther) {
    Config config1;
    Config config2(config1);

    ASSERT_EQ(config2.size(), 0);
}


TEST(ConstructorFromConfig, OneValueOther) {
    Json::Value json;
    json["key"] = 0;

    Config config1(json);
    Config config2(config1);

    ASSERT_EQ(config2.size(), 1);
}



// =============================== OPERATORS ===============================


TEST(EqualOperator, EqualConfigs) {
    Config config1;
    Config config2;

    ASSERT_TRUE(config1 == config2);
}


TEST(EqualOperator, NotEqualConfigs) {
    Json::Value json;
    json["key"] = 0;

    Config config1(json);
    Config config2;

    ASSERT_FALSE(config1 == config2);
}


TEST(EqualOperator, CompareSelf) {
    Config config1;

    ASSERT_TRUE(config1 == config1);
}


TEST(AssignOperator, AssignSelf) {
    Config config1;
    config1 = config1;

    ASSERT_TRUE(config1 == config1);
}


TEST(AssignOperator, AssignEmptyOther) {
    Config config1;
    Config config2;

    config1 = config2;

    ASSERT_TRUE(config1 == config2);
}


TEST(AssignOperator, AssignOtherWithOneValue) {
    Json::Value json;
    json["key"] = 0;

    Config config1(json);
    Config config2;

    config2 = config1;

    ASSERT_TRUE(config1 == config2);
}


TEST(AssignOperator, AssignSelfWithOneValue) {
    Json::Value json;
    json["key"] = 0;

    Config config1(json);

    config1 = config1;

    ASSERT_TRUE(config1 == config1);
}


TEST(AssignOperator, AssignJson) {
    Json::Value json;
    json["key1"] = 0;
    json["key2"] = 0;

    Config config1;

    config1 = json;

    ASSERT_EQ(config1.size(), 2);
}


TEST(AssignOperator, MoveAssignConfig) {
    Json::Value json;
    json["key1"] = 0;

    Config config1(json);
    Config config2;

    config2 = std::move(config1);

    ASSERT_EQ(config1.size(), 0);
    ASSERT_EQ(config2.size(), 1);
}




// =============================== ACCESS METHODS ===============================


TEST(SetMethod, SetEmptyKey) {
    Config config;
    config.set("", 5);

    ASSERT_EQ(config.size(), 0);
}


TEST(SetMethod, SetNotEmptyKey) {
    Config config;
    config.set("key", 5);

    ASSERT_EQ(config.size(), 1);
}


TEST(SetMethod, SetExistingEmptyKey) {
    Config config;
    config.set("key", 5);
    config.set("key", 5);

    ASSERT_EQ(config.size(), 1);
}


TEST(GetMethod, GetExistingValue) {
    Config config;
    config.set("key", 5);

    ASSERT_EQ(config.get<int>("key").value(), 5);
}


TEST(GetMethod, GetNotExistingValue) {
    Config config;
    config.set("key", 5);

    ASSERT_TRUE(!config.get<int>("wrong key").has_value());
}


TEST(GetSubconfigMethod, GetExistingEmptySubconfig) {
    Config subconfig;

    Config config;
    config.set("subconfig", subconfig);

    ASSERT_EQ(config.size(), 1);
    ASSERT_TRUE(config.get<Config>("subconfig").has_value());

    // Get subconfig
    ASSERT_TRUE(config.get_subconfig("subconfig").has_value());
}


TEST(GetSubconfigMethod, GetExistingSubconfig) {
    Config subconfig;
    subconfig.set("key", std::string("abc"));

    Config config;
    config.set("subconfig", subconfig);

    ASSERT_EQ(config.size(), 1);
    ASSERT_TRUE(config.get<Config>("subconfig").has_value());
    ASSERT_EQ(config.get<Config>("subconfig").value().get<std::string>("key"), "abc");

    // Get subconfig
    ASSERT_EQ(config.get_subconfig("subconfig").value().get<std::string>("key"), "abc");
}


TEST(GetLambda, GetExistingValue) {
    Config config;
    config.set("key", 10);

    config.get<int>("key", [](const std::optional<int>& value){
        EXPECT_TRUE(value.has_value());
        EXPECT_EQ(value.value(), 10);
    });
}


TEST(GetLambda, GetNotExistingValue) {
    Config config;
    config.set("key", 10);

    config.get<int>("wrong key", [](const std::optional<int>& value){
        EXPECT_FALSE(value.has_value());
    });
}


TEST(GetLambda, GetValueExceptionInside) {
    Config config;
    config.set("key", 10);

    try {
        config.get<int>("key", [](const std::optional<int>& value){
            throw 0;
        });
        SUCCEED();
    } catch (...) {
        FAIL();
    }
}


TEST(GetLambda, GetJson) {
    Config config;
    config.set("value", 5);

    config.get([&](const Json::Value& value){
        Json::Value json;
        json["value"] = 5;

        EXPECT_EQ(json, value);
    });
}


TEST(GetLambda, GetJsonExceptionInside) {
    Config config;

    try {
        config.get([](const Json::Value& value){
            throw 0;
        });
        SUCCEED();
    } catch (...) {
        FAIL();
    }
}


TEST(GetJson, JsonWithValue) {
    Config config;
    config.set("key", 10);

    Json::Value json;
    json["key"] = 10;

    EXPECT_EQ(json, config.get_json());
}


// =============================== OTHER ===============================

TEST(Size, Empty) {
    Config config;
    EXPECT_EQ(config.size(), 0);
}


TEST(Size, One) {
    Config config;
    config.set("key", 10);
    EXPECT_EQ(config.size(), 1);
}


TEST(Empty, True) {
    Config config;
    EXPECT_TRUE(config.empty());
}


TEST(Empty, False) {
    Config config;
    config.set("key", 10);
    EXPECT_FALSE(config.empty());
}


