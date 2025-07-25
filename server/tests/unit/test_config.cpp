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

