/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/system_file_reader.hpp
 * @brief File with a class containing helper methods for reading and processing data from files in
 * directories such as `/sys` and `/proc`.
 */

#pragma once

#include <expected> // for std::expected
#include <string>   // for std::string
#include <vector>   // for std::vector

namespace smu_server {

/**
 * @brief A class with helper methods for reading and processing data from files in directories such
 * as `/sys` and `/proc`. This class is very useful for collecting metrics that are represented as
 * files in `/proc` or `/sys`.
 */
class SystemFileReader {
  public:
    /**
     * @brief Error codes that may occur when calling various methods of this class
     * @note Use the `SystemFileReader::error_description` method to convert the error code into a
     * text description.
     */
    enum class [[nodiscard]] ErrorCode {
        OK,                 ///< No error. Success
        FILE_ACCESS_DENIED, ///< No permission to access the file
        FILE_DOESNT_EXIST,  ///< The file does not exist.
        FILE_UNKNOWN_ERROR, ///< Unknown error when working with the file
        FILE_IS_NOT_A_FILE, ///< The file is not a file. For example, it is a directory.
        FILE_READ_ERROR     ///< Error reading file
    };

    /**
     * @brief Return a human-readable description for an `ErrorCode`.
     *
     * This function returns a constant `std::string_view` describing the given
     * error code. It's cheap and can be safely used in logging and diagnostics.
     *
     * @param error The `SystemFileReader::ErrorCode` to describe.
     * @return `const std::string_view` with human-readable description (literal).
     *
     * @section example_usage Example usage:
     * @code{.cpp}
     * SystemFileReader::error_description(SystemFileReader::ErrorCode::OK); // "success"
     * SystemFileReader::error_description(SystemFileReader::ErrorCode::FILE_DOESNT_EXIST);
     * // "the requested file does not exist"
     * @endcode
     */
    static const std::string_view error_description(ErrorCode error);

    /**
     * @brief Read a text file into a `std::string`.
     *
     * This function opens the file at \p path, reads its entire content and returns it as
     * a std::string. It's intended for small text files typically found under system
     * pseudo-filesystems such as /proc and /sys.
     *
     * @note This function does not attempt to interpret the file content; it simply returns
     * raw text. Semantic parsing should be done in the caller (module) using the utilities
     * from this class.
     *
     * @param path Path to the file. Accepts any string-like path (`std::string_view`).
     *             The caller must keep the backing storage alive during the call.
     * @return `std::expected<std::string, ErrorCode>`
     *         - On **success**: expected contains the file contents as `std::string`.
     *         - On **failure**: expected contains an `ErrorCode` describing the error.
     *
     * @section example_usage Example usage:
     * @code{.cpp}
     * auto res = SystemFileReader::read_text_file("/proc/meminfo");
     * if (!res) {
     *     auto err = res.error();
     *     logger().log_error("read_text_file failed: {} ({})",
     *                        SystemFileReader::error_description(err),
     *                        static_cast<int>(err));
     * } else {
     *     auto content = *res;
     *     // parse content with your module's parser
     * }
     * @endcode
     */
    static std::expected<std::string, ErrorCode> read_text_file(const std::string_view path);

    /**
     * @brief Split the string by delimiter
     * @param str The string to be splitted
     * @param delimiter Delimiter. It can be either a single character or a string. Space by default
     * @return `std::vector<std::string_view>` with a splitted substrings.
     *
     * @note The function correctly splits the string, even if it contains **several separators in a
     * row**.
     *
     * @section example_usage Example usage:
     * @code{.cpp}
     * auto tokens = SystemFileReader::split_string("word1 word2 word3");
     * @endcode
     *
     * As a result, `tokens` will contain `{"word1", "word2", "word3"}`.
     */
    static std::vector<std::string_view> split_string(std::string_view       str,
                                                      const std::string_view delimiter = " ");

    /**
     * @brief Removes leading and trailing whitespace characters from the given string view.
     *
     * This function scans the input string view and returns a new view that excludes
     * all leading and trailing whitespace characters. The operation does not allocate
     * memory or modify the underlying string; it only adjusts the view boundaries.
     *
     * Whitespace characters are defined as: space (' '), tab (`\t`), newline (`\n`),
     * carriage return (`\r`), form feed (`\f`), and vertical tab (`\v`).
     *
     * @note The returned view references the original data, therefore the caller must ensure
     * that the underlying buffer outlives the returned view.
     *
     * @param str Input string view to be trimmed.
     * @return `std::string_view` A view representing the trimmed substring. If the input
     *         contains only whitespace, an empty string view is returned.
     *
     * @note The returned view is not null-terminated. Use it carefully with APIs that
     *       require C-style strings.
     *
     * @par Example usage
     * @code
     * std::string s = "   hello world   ";
     * std::string_view trimmed = SystemFileReader::trim(s);
     * // trimmed == "hello world"
     *
     * // Leading-only whitespace
     * std::string_view a = SystemFileReader::trim("   abc");
     * // a == "abc"
     *
     * // Trailing-only whitespace
     * std::string_view b = SystemFileReader::trim("abc   ");
     * // b == "abc"
     *
     * // Whitespace-only
     * std::string_view c = SystemFileReader::trim("   \t\n   ");
     * // c is empty: c.size() == 0
     *
     * // Beware: the returned view refers to the original buffer
     * const char* raw = "   test   ";
     * auto t = SystemFileReader::trim(raw);
     * // Modifying or destroying 'raw' invalidates 't'
     * @endcode
     */
    static std::string_view trim(std::string_view str);

    /**
     * @brief Returns the substring located before the **first occurrence of the delimiter**.
     *
     * This function searches for the first occurrence of the specified delimiter inside
     * the input string view. If the delimiter is found, a new view referencing the
     * characters before the delimiter is returned. If the delimiter is not found,
     * the entire input view is returned unchanged.
     *
     * No allocations are performed; the returned value is a view into the original
     * character buffer. The caller must ensure that the underlying storage remains
     * valid for the lifetime of the returned view.
     *
     * @param str Input string view to scan.
     * @param delimiter Delimiter to search for. Must not be empty.
     *
     * @return `std::string_view` Substring before the first occurrence of @p delimiter.
     *         If @p delimiter is empty, the behavior is undefined. If @p str is empty,
     *         an empty view is returned.
     *
     * @par Example usage
     * @code
     * std::string s = "key=value";
     * std::string_view v = SystemFileReader::token_before(s, "=");
     * // v == "key"
     *
     * // Delimiter not found
     * auto x = SystemFileReader::token_before("abc", ":");
     * // x == "abc"
     *
     * // Multiple characters as delimiter
     * auto y = SystemFileReader::token_before("path::to::file", "::");
     * // y == "path"
     *
     * // Empty input
     * auto z = SystemFileReader::token_before("", "=");
     * // z is empty
     *
     * // Be careful: returned view refers to the original buffer
     * const char* raw = "hello world";
     * auto t = SystemFileReader::token_before(raw, " ");
     * // t == "hello"
     * @endcode
     */
    static std::string_view token_before(std::string_view str, const std::string_view delimiter);


    /**
     * @brief Returns the substring located after the **first occurrence of the delimiter**.
     *
     * This function searches for the first occurrence of the specified delimiter in the
     * input string view. If the delimiter is found, the function returns a view that
     * references the characters after the delimiter. If the delimiter is not found,
     * an empty view is returned.
     *
     * No memory allocations are performed; the returned value is a view into the original
     * buffer. The caller must ensure that the original character storage remains valid
     * as long as the returned string view is used.
     *
     * @param str Input string view to scan.
     * @param delimiter Delimiter to search for. Must not be empty.
     *
     * @return `std::string_view` Substring after the first occurrence of the delimiter.
     *         Returns an empty view if the delimiter is not found or if it appears at
     *         the end of the input string. Behavior is undefined if @p delimiter is empty.
     *
     * @par Example usage
     * @code
     * std::string s = "key=value";
     * auto v = SystemFileReader::token_after(s, "=");
     * // v == "value"
     *
     * // Delimiter not found
     * auto x = SystemFileReader::token_after("abc", ":");
     * // x is empty
     *
     * // Multiple characters as delimiter
     * auto y = SystemFileReader::token_after("path::to::file", "::");
     * // y == "to::file"
     *
     * // Delimiter at the beginning
     * auto z = SystemFileReader::token_after("=start", "=");
     * // z == "start"
     *
     * // Delimiter at the end
     * auto t = SystemFileReader::token_after("end=", "=");
     * // t is empty
     * @endcode
     */
    static std::string_view token_after(std::string_view str, const std::string_view delimiter);
};

}; // namespace smu_server
