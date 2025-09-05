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
void val_sort_indices(uint32_t *a, uint32_t *b);
void val_handle_reboot_result(uint32_t test_progress);
void val_update_regression_report(uint32_t test_result, regre_report_t *regre_report);
void val_print_regression_report(regre_report_t *regre_report);

/*
 * Safe memory copy with overlap handling (memmove semantics).
 * Copies up to the minimum of `len` and `dest_sz` bytes from `src` to `dest`.
 * Returns the number of bytes actually copied. If either pointer is NULL or
 * `dest_sz` is zero, no bytes are copied and 0 is returned.
 */
size_t val_mem_copy(char *dest, size_t dest_sz, const char *src, size_t len);

#endif /* VAL_COMMON_FRAMEWORK_H */
