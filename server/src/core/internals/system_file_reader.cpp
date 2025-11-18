/// GPLv3 LICENSE, Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
/// See LICENSE for details

/**
 * @file core/internals/system_file_reader.cpp
 * @brief File with a class containing helper methods for reading and processing data from files in
 * directories such as `/sys` and `/proc`.
 */

#include <cerrno> // for errno
#include <core/internals/system_file_reader.hpp>
#include <fcntl.h>    // for open
#include <ranges>     // for std::views::split
#include <sys/stat.h> // for fstat
#include <unistd.h>   // for close
#include <vector>     // for std::vector

namespace smu_server {

// Public method
const std::string_view SystemFileReader::error_description(ErrorCode error) {
    switch (error) {
    case ErrorCode::OK:
        return "success";
    case ErrorCode::FILE_DOESNT_EXIST:
        return "the requested file does not exist";
    case ErrorCode::FILE_ACCESS_DENIED:
        return "error accessing the requested file (no permissions)";
    case ErrorCode::FILE_UNKNOWN_ERROR:
        return "unknown error when working with the requested file";
    case ErrorCode::FILE_IS_NOT_A_FILE:
        return "the requested file is not a file";
    case ErrorCode::FILE_READ_ERROR:
        return "file read error";
    default:
        return "unknown error code";
    }
}


// Public method
std::expected<std::string, SystemFileReader::ErrorCode> SystemFileReader::read_text_file(
    const std::string_view path) {

    // Open file
    int fd = ::open(path.data(), O_RDONLY);
    if (fd == -1) {
        int en = errno;
        if (en == EACCES) {
            return std::unexpected(ErrorCode::FILE_ACCESS_DENIED);
        }
        if (en == ENOENT) {
            return std::unexpected(ErrorCode::FILE_DOESNT_EXIST);
        } else {
            return std::unexpected(ErrorCode::FILE_UNKNOWN_ERROR);
        }
    }

    // Try to get filesize
    struct stat st;
    if (fstat(fd, &st) != 0) {
        ::close(fd);
        return std::unexpected(ErrorCode::FILE_UNKNOWN_ERROR);
    }

    if (S_ISDIR(st.st_mode)) {
        ::close(fd);
        return std::unexpected(ErrorCode::FILE_IS_NOT_A_FILE);
    }

    // Output string
    std::string out;

    if (st.st_size > 0) {
        out.reserve(static_cast<std::size_t>(st.st_size));
    } else {
        // unknown size (e.g. /proc pseudo-files), start with modest reserve
        out.reserve(4096);
    }

    // Reading
    constexpr std::size_t CHUNK = 4096; // chunk size
    std::vector<char>     buf(CHUNK);
    while (true) {
        ssize_t r = ::read(fd, buf.data(), buf.size());
        // error
        if (r < 0) {
            int en = errno;
            if (en == EINTR) {
                continue;
            }
            ::close(fd);
            return std::unexpected(ErrorCode::FILE_READ_ERROR);
        }
        // EOF
        if (r == 0) {
            break;
        }
        out.append(buf.data(), static_cast<std::size_t>(r));
    }

    ::close(fd);
    return out;
}


// Public method
std::vector<std::string_view> SystemFileReader::split_string(std::string_view       str,
                                                             const std::string_view delimiter) {
    if (str.empty()) {
        return std::vector<std::string_view>();
    }

    std::vector<std::string_view> result;
    result.reserve(10); // For most lines, this is sufficient.

    auto view = str | std::views::split(delimiter);

    for (const auto& substr : view) {
        if (substr.empty())
            continue;
        result.emplace_back(substr.begin(), substr.end());
    }

    return result;
}


// Public method
std::string_view SystemFileReader::trim(std::string_view str) {
    auto begin = str.find_first_not_of(" \t\n\r\f\v");

    if (begin == std::string_view::npos) {
        return {};
    }

    auto end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(begin, end - begin + 1);
}


// Public method
std::string_view SystemFileReader::token_before(std::string_view       str,
                                                const std::string_view delimiter) {
    auto end = str.find_first_of(delimiter);

    if (end == std::string_view::npos) {
        return str;
    }

    return str.substr(0, end);
}


// Public method
std::string_view SystemFileReader::token_after(std::string_view       str,
                                               const std::string_view delimiter) {
    auto begin = str.find(delimiter);

    if (begin == std::string_view::npos) {
        return {};
    }

    return str.substr(begin + delimiter.size(), str.size() - begin + 1);
}


}; // namespace smu_server
