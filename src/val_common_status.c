/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_common_status.h"
#include "val_common_log.h"
#include <inttypes.h>
#include <stdatomic.h>
#include <limits.h>

static _Atomic void *shared_region_base;
static const uint32_t test_status_mask = ((uint32_t)TEST_STATE_MASK << TEST_STATE_SHIFT) |
                                         TEST_STATUS_CODE_MASK;

static inline val_test_status_buffer_ts *val_status_buffer(void)
{
    return (val_test_status_buffer_ts *)((uint8_t *)val_get_shared_region_base()
                                         + TEST_STATUS_OFFSET);
}

/**
 *   @brief    Returns the IPA address of the shared region
 *   @param    ipa_width      - Realm IPA width
 *   @return   IPA address of the shared region
**/
void *val_base_addr_ipa(uint64_t ipa_width)
{
    uint64_t ipa_bit = 0;

    if (ipa_width != 0) {
        uint64_t max_shift = (sizeof(uint64_t) * CHAR_BIT) - 1;
        uint64_t shift = ipa_width - 1;

        if (shift > max_shift)
            shift = max_shift;

        ipa_bit = 1ull << shift;
    }

    void *ipa_base = (void *)(uintptr_t)(VAL_NS_SHARED_REGION_IPA_OFFSET | ipa_bit);

    atomic_store_explicit(&shared_region_base, ipa_base, memory_order_release);

    return ipa_base;
}

/**
 *   @brief    Returns the base address of the shared region
 *   @param    Void
 *   @return   Physical address of the shared region
**/
void *val_get_shared_region_base_pa(void)
{
    return ((void *)(PLATFORM_SHARED_REGION_BASE));
}

/**
 *   @brief    Returns the base address of the shared region
 *   @param    Void
 *   @return   Base address of the shared region
**/
void *val_get_shared_region_base(void)
{
    void *ipa_base = atomic_load_explicit(&shared_region_base, memory_order_acquire);

    if (ipa_base)
        return ipa_base;

    return val_get_shared_region_base_pa();
}

/**
 *   @brief    Records the state and status of test
 *   @param    status - Test status bit field - (state|status_code)
 *   @return   void
**/
void val_set_status(uint32_t status)
{
    val_test_status_buffer_ts *curr_test_status = val_status_buffer();
    uint32_t expected = atomic_load_explicit(&curr_test_status->word, memory_order_relaxed);
    uint32_t desired;

    do {
        desired = (expected & ~test_status_mask) | (status & test_status_mask);
    } while (!atomic_compare_exchange_weak_explicit(&curr_test_status->word,
                                                    &expected,
                                                    desired,
                                                    memory_order_release,
                                                    memory_order_relaxed));
}

/**
 *   @brief    Returns the state and status for a given test
 *   @param    Void
 *   @return   test status
**/
uint32_t val_get_status(void)
{
    val_test_status_buffer_ts *curr_test_status = val_status_buffer();

    return atomic_load_explicit(&curr_test_status->word, memory_order_acquire) & test_status_mask;
}

/**
 *   @brief    Parses input status for a given test and
 *             outputs appropriate information on the console
 *   @param    Void
 *   @return   Test state
**/
uint32_t val_report_status(void)
{
    uint32_t status, status_code, state;
    status = val_get_status();
    state = (status >> TEST_STATE_SHIFT) & TEST_STATE_MASK;
    status_code = status & TEST_STATUS_CODE_MASK;

    switch (state)
    {
        case TEST_PASS:
            state = TEST_PASS;
            val_printf(ALWAYS, "Result=Passed\n");
            break;

        case TEST_FAIL:
            state = TEST_FAIL;
            val_printf(ALWAYS, "Result=Failed (Error code=%" PRIu32 ")\n",
                status_code);
            break;

        case TEST_SKIP:
            state = TEST_SKIP;
            val_printf(ALWAYS, "Result=Skipped (Skip code=%" PRIu32 ")\n",
                status_code);
            break;

        case TEST_ERROR:
            state = TEST_ERROR;
            val_printf(ALWAYS, "Result=Error (Error code=%" PRIu32 ")\n",
                status_code);
            break;
        default:
            state = TEST_FAIL;
            val_printf(ALWAYS, "Result=Failed (Error Code=%" PRIu32 ")\n",
                status_code);
            break;
    }

    val_printf(ALWAYS, "***********************************\n");
    return state;
}
