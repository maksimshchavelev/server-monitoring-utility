/**
 * @file utils.h
 * @brief Utils for UPTIME module
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include <stddef.h> // For size_t
#include <stdint.h> // For uint64_t

/**
 * @brief Reads `proc/uptime` and writes the result to `out_uptime` and `out_idle`
 * @param out_uptime The variable to which uptime will be written
 * @param out_idle The variable into which idle will be written (see
 * https://www.site24x7.com/learn/linux/uptime.html#:~:text=The%20%2Fproc%20directory%20contains%20a,the%20machine%20has%20been%20idle)
 * @return `0` on success and `-1` on error (`errno` is set)
 */
int utils_read_proc_uptime(double *out_uptime, double *out_idle);


/**
 * @brief Converts seconds to `dd:hh:mm:ss` time format
 * @param seconds Seconds
 * @param buffer The buffer to write the result to
 * @param buffer_size Size of buffer
 * @return `0` on success and `-1` on error (`errno` is set)
 */
int utils_seconds_to_dd_hh_mm_ss(uint64_t seconds, char* buffer, size_t buffer_size);
