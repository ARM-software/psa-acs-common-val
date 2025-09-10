/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef VAL_COMMON_FRAMEWORK_H
#define VAL_COMMON_FRAMEWORK_H

#include "val_common.h"

typedef struct {
    uint32_t suite_num;
    uint32_t test_num;
    uint32_t end_test_num;
    uint32_t test_progress;
} test_info_t;

typedef struct {
    uint32_t total_pass;
    uint32_t total_fail;
    uint32_t total_skip;
    uint32_t total_error;
} regre_report_t;


void val_log_test_info(test_info_t *test_info);
uint32_t is_reboot_run(uint32_t test_progress, const uint8_t *pattern, uint32_t length);
void val_reset_test_info_fields(test_info_t *test_info);
void val_reset_regression_report(regre_report_t *report);
void val_log_final_test_status(test_info_t *test_info, regre_report_t *regre_report);
/*
 * Sort two indices with explicit bounds.
 * - Ensures the two indices are ordered (a <= b) and within [0, max_index].
 * - If either index is out of range, it is clamped to the nearest valid value.
 * - Returns 0 if inputs were already valid and ordered, 1 if clamping or reordering occurred,
 *   and -1 if the pointers are NULL (no changes performed).
 */
int val_sort_indices_bounded(uint32_t *a, uint32_t *b, uint32_t max_index);

/*
 * Deprecated: Unbounded variant is ambiguous and can lead to misuse.
 * Prefer 'val_sort_indices_bounded'. This wrapper maintains ABI but does not enforce bounds.
 */
#if defined(__GNUC__) || defined(__clang__)
#define VAL_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define VAL_DEPRECATED(msg)
#endif
void VAL_DEPRECATED("Use val_sort_indices_bounded() with an explicit max_index")
val_sort_indices(uint32_t *a, uint32_t *b);
void val_handle_reboot_result(uint32_t test_progress);
void val_update_regression_report(uint32_t test_result, regre_report_t *regre_report);
void val_print_regression_report(regre_report_t *regre_report);

/*
 * Bounded, overlap-safe memory copy.
 * Copies up to the minimum of 'len' and 'dest_size' bytes from 'src' to 'dest'.
 * Behaves like memmove for overlapping regions.
 * Returns the number of bytes copied.
 */
size_t val_mem_copy(char *dest, size_t dest_size, const char *src, size_t len);

#endif /* VAL_COMMON_FRAMEWORK_H */
