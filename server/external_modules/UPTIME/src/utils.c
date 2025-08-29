/**
 * @file utils.c
 * @brief Utils for UPTIME module
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "utils.h"
#include <stdio.h> // For file operations


int utils_read_proc_uptime(double *out_uptime, double *out_idle)
{
    FILE *file = fopen("/proc/uptime", "r");

    // Error
    if (!file) {
        return -1; // errno is set
    }

    // Error
    if (fscanf(file, "%lf %lf", out_uptime, out_idle) != 2) {
        fclose(file);
        return -1; // errno is set
    }

    fclose(file);
    return 0; // success
}


int utils_seconds_to_dd_hh_mm_ss(uint64_t seconds, char *buffer, size_t buffer_size)
{
    uint64_t days = seconds / 86400;
    seconds %= 86400;

    uint64_t hours = seconds / 3600;
    seconds %= 3600;

    uint64_t minutes = seconds / 60;
    seconds %= 60;

    // Error
    if (snprintf(buffer, buffer_size, "%02lu:%02lu:%02lu:%02lu", days, hours, minutes, seconds) < 0) {
        return -1; // errno is set
    }

    return 0; // success
}

