/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @brief Test for testing core/internals/system_file_reader.hpp
 */

#include "core/internals/system_file_reader.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace smu_server;

using Reader = SystemFileReader;
using ErrorCode = SystemFileReader::ErrorCode;

// Directory with samples
const std::string samples = UNIT_TESTS_SAMPLES_DIR + std::string("/test_system_file_reader/");


// ====================== READ_TEXT_FILE ======================

// --------- Testing reading an empty text file ---------

TEST(system_file_reader, read_empty_text_file) {
    auto res = Reader::read_text_file(samples + "empty");
    EXPECT_TRUE(res.has_value());
    EXPECT_TRUE(res.value().empty());
}

// --------- Testing reading an text file with `abc` data ---------

TEST(system_file_reader, read_one_line_text_file) {
    auto res = Reader::read_text_file(samples + "abc");
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "abc\n"); // According to POSIX, strings end with \n
}

// --------- Testing reading an text file with `abc` `def` data in 2 lines ---------

TEST(system_file_reader, read_two_lines_text_file) {
    auto res = Reader::read_text_file(samples + "abc_def_2_lines");
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "abc\ndef\n");
}

// --------- Testing reading an text file with no permissions ---------

TEST(system_file_reader, read_file_with_no_permissions) {
    // We need to create temporary file with no permissions
    namespace fs = std::filesystem;
    fs::path temp_file = fs::temp_directory_path() / "no_perms_test";

    {
        std::ofstream out(temp_file);
        out << "content";
    }

    fs::permissions(temp_file, fs::perms::none);

    auto res = Reader::read_text_file(fs::temp_directory_path().string() + "/no_perms_test");
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ErrorCode::FILE_ACCESS_DENIED);
}

// --------- Testing reading an non existing text file ---------

TEST(system_file_reader, read_non_existing_file) {
    auto res = Reader::read_text_file(samples + "404");
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ErrorCode::FILE_DOESNT_EXIST);
}

// --------- Testing reading an directory instead of file ---------

TEST(system_file_reader, read_directory) {
    auto res = Reader::read_text_file(samples + "not_a_file");
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ErrorCode::FILE_IS_NOT_A_FILE);
}



// ====================== ERROR_DESCRIPTION ======================

// --------- No error ---------
TEST(system_file_reader, error_description_ok) {
    EXPECT_EQ(Reader::error_description(ErrorCode::OK), "success");
}

// --------- File doesn't exist ---------
TEST(system_file_reader, error_description_file_doesnt_exist) {
    EXPECT_EQ(Reader::error_description(ErrorCode::FILE_DOESNT_EXIST),
              "the requested file does not exist");
}

// --------- Access denied ---------
TEST(system_file_reader, error_description_access_denied) {
    EXPECT_EQ(Reader::error_description(ErrorCode::FILE_ACCESS_DENIED),
              "error accessing the requested file (no permissions)");
}

// --------- Unknown error ---------
TEST(system_file_reader, error_description_file_unknown_error) {
    EXPECT_EQ(Reader::error_description(ErrorCode::FILE_UNKNOWN_ERROR),
              "unknown error when working with the requested file");
}

// --------- Unknown error ---------
TEST(system_file_reader, error_description_file_is_not_a_file) {
    EXPECT_EQ(Reader::error_description(ErrorCode::FILE_IS_NOT_A_FILE),
              "the requested file is not a file");
}

// --------- File read error ---------
TEST(system_file_reader, error_description_file_read_error) {
    EXPECT_EQ(Reader::error_description(ErrorCode::FILE_READ_ERROR),
              "file read error");
}



// ====================== SPLIT_STRING ======================

// --------- Split empty string ---------

TEST(system_file_reader, split_empty_string) {
    auto res = Reader::split_string("");
    EXPECT_TRUE(res.empty());
}

// --------- Split string with 3 words ---------

TEST(system_file_reader, split_string_with_3_words) {
    auto res = Reader::split_string("word1 word2 word3");
    EXPECT_EQ(res.size(), 3);
    EXPECT_EQ(res[0], "word1");
    EXPECT_EQ(res[1], "word2");
    EXPECT_EQ(res[2], "word3");
}

// --------- All string is from delimiters ---------

TEST(system_file_reader, split_string_from_delimiters) {
    auto res = Reader::split_string("    ");
    EXPECT_TRUE(res.empty());
}

// --------- Split a string surrounded by spaces ---------

TEST(system_file_reader, split_string_surrounded_with_spaces) {
    auto res = Reader::split_string("   string   ");
    EXPECT_EQ(res.size(), 1);
    EXPECT_EQ(res[0], "string");
}

// --------- Split a string with multiple words and spaces ---------

TEST(system_file_reader, split_string_with_multiple_words_and_spaces) {
    auto res = Reader::split_string("   word1     word2   word3");
    EXPECT_EQ(res.size(), 3);
    EXPECT_EQ(res[0], "word1");
    EXPECT_EQ(res[1], "word2");
    EXPECT_EQ(res[2], "word3");
}

// --------- Split a string with multiple words and custom delimiter ---------

TEST(system_file_reader, split_string_with_multiple_words_and_custom_delimiter) {
    auto res = Reader::split_string("DELIMITERDELIMITERword1DELIMITERword2DELIMITERword3 ", "DELIMITER");
    EXPECT_EQ(res.size(), 3);
    EXPECT_EQ(res[0], "word1");
    EXPECT_EQ(res[1], "word2");
    EXPECT_EQ(res[2], "word3 ");
}




// ====================== TRIM ======================

// --------- Trim a string surrounded by spaces ---------

TEST(system_file_reader, trim_spaces) {
    auto res = Reader::trim("   trim   ");
    EXPECT_EQ(std::string(res), "trim");
    EXPECT_EQ(res.size(), 4);
}

// --------- Trim a hard string surrounded by all possible characters ---------

TEST(system_file_reader, trim_hard_string) {
    auto res = Reader::trim("\t\n\r\f\v   trim1  trim2   \t\n\r\f\v");
    EXPECT_EQ(std::string(res), "trim1  trim2");
    EXPECT_EQ(res.size(), 12);
}



// ====================== TOKEN_BEFORE ======================

TEST(system_file_reader, token_before_basic) {
    auto res = Reader::token_before("key=value", "=");
    EXPECT_EQ(res, "key");
}

TEST(system_file_reader, token_before_no_delimiter) {
    auto res = Reader::token_before("abc", ":");
    EXPECT_EQ(res, "abc");
}

TEST(system_file_reader, token_before_multi_char_delimiter) {
    auto res = Reader::token_before("path::to::file", "::");
    EXPECT_EQ(res, "path");
}

TEST(system_file_reader, token_before_empty_input) {
    auto res = Reader::token_before("", "=");
    EXPECT_TRUE(res.empty());
}

TEST(system_file_reader, token_before_delimiter_at_start) {
    auto res = Reader::token_before("=value", "=");
    EXPECT_TRUE(res.empty());
}

TEST(system_file_reader, token_before_delimiter_at_end) {
    auto res = Reader::token_before("value=", "=");
    EXPECT_EQ(res, "value");
}

TEST(system_file_reader, token_before_multiple_delimiters) {
    auto res = Reader::token_before("a=b=c", "=");
    EXPECT_EQ(res, "a");
}

TEST(system_file_reader, token_before_empty_delimiter_undefined_behavior) {
    EXPECT_NO_FATAL_FAILURE({
        auto res = Reader::token_before("abc", "");
        (void)res;
    });
}



// ====================== TOKEN_AFTER ======================

TEST(system_file_reader, token_after_basic) {
    auto res = Reader::token_after("key=value", "=");
    EXPECT_EQ(res, "value");
}

TEST(system_file_reader, token_after_no_delimiter) {
    auto res = Reader::token_after("abc", ":");
    EXPECT_TRUE(res.empty());
}

TEST(system_file_reader, token_after_multi_char_delimiter) {
    auto res = Reader::token_after("path::to::file", "::");
    EXPECT_EQ(res, "to::file");
}

TEST(system_file_reader, token_after_empty_input) {
    auto res = Reader::token_after("", "=");
    EXPECT_TRUE(res.empty());
}

TEST(system_file_reader, token_after_delimiter_at_start) {
    auto res = Reader::token_after("=value", "=");
    EXPECT_EQ(res, "value");
}

TEST(system_file_reader, token_after_delimiter_at_end) {
    auto res = Reader::token_after("value=", "=");
    EXPECT_TRUE(res.empty());
}

TEST(system_file_reader, token_after_multiple_delimiters) {
    auto res = Reader::token_after("a=b=c", "=");
    EXPECT_EQ(res, "b=c");
}


TEST(system_file_reader, token_after_empty_delimiter_undefined_behavior) {
    EXPECT_NO_FATAL_FAILURE({
        auto r = Reader::token_after("abc", "");
        (void)r;
    });
}


// ====================== TOLOWER ======================

TEST(system_file_reader, tolower_basic) {
    EXPECT_EQ(SystemFileReader::tolower("ABC"), "abc");
}

TEST(system_file_reader, tolower_mixed_case) {
    EXPECT_EQ(SystemFileReader::tolower("HeLLo"), "hello");
}

TEST(system_file_reader, tolower_already_lower) {
    EXPECT_EQ(SystemFileReader::tolower("world"), "world");
}

TEST(system_file_reader, tolower_digits_and_symbols) {
    EXPECT_EQ(SystemFileReader::tolower("123_+=!@#"), "123_+=!@#");
}

TEST(system_file_reader, tolower_empty_string) {
    EXPECT_EQ(SystemFileReader::tolower(""), "");
}

TEST(system_file_reader, tolower_long_string) {
    std::string input = "ThIs_Is_A_LoNg_StRiNg_123!!!";
    std::string expected = "this_is_a_long_string_123!!!";

    EXPECT_EQ(SystemFileReader::tolower(input), expected);
}



// ====================== CONVERT_UNITS ======================

using SizeUnit = Reader::SizeUnit;

TEST(system_file_reader, convert_units_basic)
{
    EXPECT_EQ(Reader::convert_units("b", SizeUnit::BYTES, 123), 123);
    EXPECT_EQ(Reader::convert_units("bytes", SizeUnit::BYTES, 999), 999);
}

TEST(system_file_reader, convert_units_kb_to_bytes)
{
    EXPECT_EQ(Reader::convert_units("KB", SizeUnit::BYTES, 1), 1024);
    EXPECT_EQ(Reader::convert_units("kilobytes", SizeUnit::BYTES, 2), 2048);
}

TEST(system_file_reader, convert_units_mb_to_kb)
{
    EXPECT_EQ(Reader::convert_units("MB", SizeUnit::KBYTES, 1), 1024);
    EXPECT_EQ(Reader::convert_units("megabytes", SizeUnit::KBYTES, 3), 3 * 1024);
}

TEST(system_file_reader, convert_units_gb_to_mb)
{
    EXPECT_EQ(Reader::convert_units("GB", SizeUnit::MBYTES, 1), 1024);
}

TEST(system_file_reader, convert_units_smaller_to_larger)
{
    EXPECT_EQ(Reader::convert_units("bytes", SizeUnit::KBYTES, 1024), 1);
    EXPECT_EQ(Reader::convert_units("KB", SizeUnit::MBYTES, 1024), 1);
}

TEST(system_file_reader, convert_units_unknown_unit)
{
    // Unknown - treat as bytes
    EXPECT_EQ(Reader::convert_units("banana", SizeUnit::KBYTES, 2048), 2);
}

