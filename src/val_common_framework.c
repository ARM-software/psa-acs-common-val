/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <string.h>

#include "val_common_framework.h"
#include "val_common_log.h"
#include "val_common_status.h"

extern const uint32_t total_tests;

static void val_increment_regression_counter(uint32_t *counter, const char *counter_name)
{
    uint32_t current_value = __atomic_load_n(counter, __ATOMIC_RELAXED);

    while (1)
    {
        if (current_value == UINT32_MAX)
        {
            val_printf(ERROR,
                       "Regression counter %s reached max value (%u); increment skipped\n",
                       counter_name,
                       UINT32_MAX);
            return;
        }

        if (__atomic_compare_exchange_n(counter,
                                         &current_value,
                                         current_value + 1U,
                                         1,
                                         __ATOMIC_ACQ_REL,
                                         __ATOMIC_RELAXED))
        {
            return;
        }
    }
}

/**
 *   @brief   -  Logs the test_info details
 *   @param   -  test_info : Test information struct address
 *   @return  -  void
**/
void val_log_test_info(test_info_t *test_info)
{
    val_printf(INFO, "In val_get_last_run_test_info, test_num=%x\n", test_info->test_num);
    val_printf(INFO, "suite_num=%x\n", test_info->suite_num);
    val_printf(INFO, "test_progress=%x\n", test_info->test_progress);
}

/**
 *   @brief   -  Checks if the test progress indicates a reboot run (warm reset)
 *   @param   -  test_progress : Test progress value
 *            -  pattern       : Pattern array
 *            -  length        : Length of the pattern array
 *   @return  -  1 if match found (warm reset), 0 otherwise (power-on reset)
**/
uint32_t is_reboot_run(uint32_t test_progress, const uint8_t *pattern, uint32_t length)
{
    uint32_t i;
    for (i = 0; i < length; i++)
    {
        if (test_progress == pattern[i])
            return 1;
    }
    return 0;
}

/**
 *   @brief   -  Resets 'test_info' fields for a power on reset
 *   @param   -  test_info : Test information struct address
 *   @return  -  void
**/
void val_reset_test_info_fields(test_info_t *test_info)
{
    test_info->test_num      = VAL_INVALID_TEST_NUM;
    test_info->end_test_num  = total_tests;
    test_info->suite_num     = 0;
    test_info->test_progress = 0;
}

/**
 *  @brief   -  Resets all fields in the provided 'regre_report_t' struct.
 *  @param   -  report : Regression report struct address
 *  @return  -  void
 */
void val_reset_regression_report(regre_report_t *report)
{
    if (report == NULL)
    {
        val_printf(ERROR, "Regression report pointer is NULL\n");
        return;
    }

    __atomic_store_n(&report->total_pass, 0U, __ATOMIC_RELAXED);
    __atomic_store_n(&report->total_fail, 0U, __ATOMIC_RELAXED);
    __atomic_store_n(&report->total_skip, 0U, __ATOMIC_RELAXED);
    __atomic_store_n(&report->total_error, 0U, __ATOMIC_RELAXED);
}

/**
 *   @brief   -  Logs final 'test_info' and Regression report values
 *   @param   -  test_info    : Test information struct address
 *            -  regre_report : Regression report struct
 *   @return  -  void
**/
void val_log_final_test_status(test_info_t *test_info, regre_report_t *regre_report)
{
    if (regre_report == NULL)
    {
        val_printf(ERROR, "Regression report pointer is NULL\n");
        return;
    }

    uint32_t total_pass  = __atomic_load_n(&regre_report->total_pass, __ATOMIC_RELAXED);
    uint32_t total_fail  = __atomic_load_n(&regre_report->total_fail, __ATOMIC_RELAXED);
    uint32_t total_skip  = __atomic_load_n(&regre_report->total_skip, __ATOMIC_RELAXED);
    uint32_t total_error = __atomic_load_n(&regre_report->total_error, __ATOMIC_RELAXED);

    val_printf(INFO, "In val_get_last_run_test_num, test_num=%x\n", test_info->test_num);
    val_printf(INFO, "suite_num=%x\n", test_info->suite_num);
    val_printf(INFO, "regre_report.total_pass=%x\n", total_pass);
    val_printf(INFO, "regre_report.total_fail=%x\n", total_fail);
    val_printf(INFO, "regre_report.total_skip=%x\n", total_skip);
    val_printf(INFO, "regre_report.total_error=%x\n", total_error);
}

/**
 *  @brief   -  Ensures two indices are in ascending order
 *  @param   -  a : Pointer to first index
 *           -  b : Pointer to second index
 *  @return  -  void
 */
void val_sort_indices(uint32_t *a, uint32_t *b)
{
    if (*a > *b)
    {
        uint32_t temp = *a;
        *a = *b;
        *b = temp;
    }
}

/**
 *  @brief   -  Handles Test result reporting after Reboot process
 *  @param   -  test_progress : Progress status from 'test_info'
 *  @return  -  void
 */
void val_handle_reboot_result(uint32_t test_progress)
{
    if (test_progress == TEST_REBOOTING)
    {
        /* Reboot expected, declare previous test as pass */
        val_set_status(RESULT_PASS(VAL_SUCCESS));
    }
    else
    {
        /* Reboot not expected, declare previous test as error */
        val_set_status(RESULT_ERROR(VAL_SIM_ERROR));
    }
}

/**
 *  @brief   -  Updates the Regression report based on test result
 *  @param   -  test_result   : Result of the executed test
 *           -  regre_report  : Pointer to the Regression report struct
 *  @return  -  void
 */
void val_update_regression_report(uint32_t test_result, regre_report_t *regre_report)
{
    if (regre_report == NULL)
    {
        val_printf(ERROR, "Regression report pointer is NULL\n");
        return;
    }

    switch (test_result)
    {
        case TEST_PASS:
            val_increment_regression_counter(&regre_report->total_pass, "total_pass");
            break;
        case TEST_FAIL:
            val_increment_regression_counter(&regre_report->total_fail, "total_fail");
            break;
        case TEST_SKIP:
            val_increment_regression_counter(&regre_report->total_skip, "total_skip");
            break;
        case TEST_ERROR:
            val_increment_regression_counter(&regre_report->total_error, "total_error");
            break;
    }
}

/**
 *  @brief   -  Prints the final Regression report summary
 *  @param   -  regre_report : Pointer to the Regression report struct
 *  @return  -  void
 */
void val_print_regression_report(regre_report_t *regre_report)
{
    if (regre_report == NULL)
    {
        val_printf(ERROR, "Regression report pointer is NULL\n");
        return;
    }

    uint32_t total_pass  = __atomic_load_n(&regre_report->total_pass, __ATOMIC_RELAXED);
    uint32_t total_fail  = __atomic_load_n(&regre_report->total_fail, __ATOMIC_RELAXED);
    uint32_t total_skip  = __atomic_load_n(&regre_report->total_skip, __ATOMIC_RELAXED);
    uint32_t total_error = __atomic_load_n(&regre_report->total_error, __ATOMIC_RELAXED);
    uint32_t total_tests = total_pass + total_fail + total_skip + total_error;

    val_printf(ALWAYS, "\n\n");
    val_printf(ALWAYS, "REGRESSION REPORT: \n");
    val_printf(ALWAYS, "==========================\n");
    val_printf(ALWAYS, "   TOTAL TESTS     : %d\n", total_tests, 0);
    val_printf(ALWAYS, "   TOTAL PASSED    : %d\n", total_pass);
    val_printf(ALWAYS, "   TOTAL FAILED    : %d\n", total_fail);
    val_printf(ALWAYS, "   TOTAL SKIPPED   : %d\n", total_skip);
    val_printf(ALWAYS, "   TOTAL SIM ERROR : %d\n", total_error);
    val_printf(ALWAYS, "==========================\n");
    val_printf(ALWAYS, "******* END OF ACS *******\n");
    val_printf(ALWAYS, "\n");
}

/**
 *  @brief   -  Copies up to 'len' bytes from source to destination buffer
 *  @param   -  dest       : Destination buffer
 *           -  dest_size  : Size of the destination buffer in bytes
 *           -  src        : Source buffer
 *           -  len        : Number of bytes to copy
 *  @return  -  void
 */
void val_mem_copy(char *dest, size_t dest_size, const char *src, size_t len)
{
    if (dest == NULL || src == NULL || dest_size == 0)
        return;

    size_t copy_len = len;

    if (copy_len > dest_size)
        copy_len = dest_size;

    if (copy_len == 0 || dest == src)
        return;

    memmove(dest, src, copy_len);
}
