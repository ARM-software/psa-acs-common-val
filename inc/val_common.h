/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef VAL_COMMON_H
#define VAL_COMMON_H

#include "pal_common_val_intf.h"
#include <stdint.h>

/* Various test status codes, Max value = 0xff */
#define  VAL_SUCCESS            0u
#define  VAL_ERROR_POINT(n)     (uint32_t)(n)
#define  VAL_TEST_INIT_FAILED   101u
#define  VAL_STATUS_INVALID     102u
#define  VAL_SKIP_CHECK         103u
#define  VAL_SIM_ERROR          104u

#define  VAL_STATUS_ERROR_MAX   255u
#define  VAL_INVALID_TEST_NUM   0xFFFFFFFFu

/*
 * Safe helpers to avoid multiple evaluation of macro arguments
 * and ensure well-defined behavior with proper types.
 */
static inline uint32_t val_bit_mask(uint32_t len)
{
    /* If len >= 32, return full mask; shifting a 32-bit 1 by >=32 is UB */
    if (len >= 32U) {
        return 0xFFFFFFFFu;
    }
    return (len == 0U) ? 0U : ((1u << len) - 1u);
}

static inline uint32_t val_set_bits(uint32_t data, uint32_t pos, uint32_t len, uint32_t val)
{
    /* Build mask at position and set only the relevant bits from val */
    uint32_t mask = val_bit_mask(len);

    /* Avoid undefined shift: if pos >= 32, mask_at_pos becomes 0 */
    uint32_t mask_at_pos = (pos >= 32U) ? 0U : (mask << pos);
    uint32_t val_at_pos  = (pos >= 32U) ? 0U : ((val & mask) << pos);

    return (data & ~mask_at_pos) | val_at_pos;
}

/* Backward-compatible macros that evaluate arguments exactly once */
#define VAL_BIT_MASK(len)            val_bit_mask((uint32_t)(len))
/* Set the value in given position */
#define VAL_SET_BITS(data, pos, len, val) \
    val_set_bits((uint32_t)(data), (uint32_t)(pos), (uint32_t)(len), (uint32_t)(val))


/* Test state macros */
#define TEST_START              0x01u
#define TEST_PASS               0x02u
#define TEST_FAIL               0x03u
#define TEST_SKIP               0x04u
#define TEST_ERROR              0x05u
#define TEST_END                0x06u
#define TEST_REBOOTING          0x07u

#define TEST_STATE_SHIFT        8u
#define TEST_STATE_MASK         0xFFu
#define TEST_STATUS_CODE_MASK   0xFFu
#define TEST_STATUS_CODE_SHIFT  0u

#define RESULT_START(status) \
    ( (uint32_t)(TEST_START) << (uint32_t)TEST_STATE_SHIFT | \
      (((uint32_t)(status) & (uint32_t)TEST_STATUS_CODE_MASK) << (uint32_t)TEST_STATUS_CODE_SHIFT) )
#define RESULT_END(status) \
    ( (uint32_t)(TEST_END) << (uint32_t)TEST_STATE_SHIFT | \
      (((uint32_t)(status) & (uint32_t)TEST_STATUS_CODE_MASK) << (uint32_t)TEST_STATUS_CODE_SHIFT) )
#define RESULT_PASS(status) \
    ( (uint32_t)(TEST_PASS) << (uint32_t)TEST_STATE_SHIFT | \
      (((uint32_t)(status) & (uint32_t)TEST_STATUS_CODE_MASK) << (uint32_t)TEST_STATUS_CODE_SHIFT) )
#define RESULT_FAIL(status) \
    ( (uint32_t)(TEST_FAIL) << (uint32_t)TEST_STATE_SHIFT | \
      (((uint32_t)(status) & (uint32_t)TEST_STATUS_CODE_MASK) << (uint32_t)TEST_STATUS_CODE_SHIFT) )
#define RESULT_SKIP(status) \
    ( (uint32_t)(TEST_SKIP) << (uint32_t)TEST_STATE_SHIFT | \
      (((uint32_t)(status) & (uint32_t)TEST_STATUS_CODE_MASK) << (uint32_t)TEST_STATUS_CODE_SHIFT) )
#define RESULT_ERROR(status) \
    ( (uint32_t)(TEST_ERROR) << (uint32_t)TEST_STATE_SHIFT | \
      (((uint32_t)(status) & (uint32_t)TEST_STATUS_CODE_MASK) << (uint32_t)TEST_STATUS_CODE_SHIFT) )

#define IS_TEST_FAIL(status)    ((((uint32_t)(status) >> (uint32_t)TEST_STATE_SHIFT) & (uint32_t)TEST_STATE_MASK) == (uint32_t)TEST_FAIL)
#define IS_TEST_PASS(status)    ((((uint32_t)(status) >> (uint32_t)TEST_STATE_SHIFT) & (uint32_t)TEST_STATE_MASK) == (uint32_t)TEST_PASS)
#define IS_TEST_SKIP(status)    ((((uint32_t)(status) >> (uint32_t)TEST_STATE_SHIFT) & (uint32_t)TEST_STATE_MASK) == (uint32_t)TEST_SKIP)
#define IS_TEST_ERROR(status)   ((((uint32_t)(status) >> (uint32_t)TEST_STATE_SHIFT) & (uint32_t)TEST_STATE_MASK) == (uint32_t)TEST_ERROR)
#define IS_TEST_START(status)   ((((uint32_t)(status) >> (uint32_t)TEST_STATE_SHIFT) & (uint32_t)TEST_STATE_MASK) == (uint32_t)TEST_START)
#define IS_TEST_END(status)     ((((uint32_t)(status) >> (uint32_t)TEST_STATE_SHIFT) & (uint32_t)TEST_STATE_MASK) == (uint32_t)TEST_END)
#define IS_STATUS_FAIL(status)  ((((uint32_t)(status)) & (uint32_t)TEST_STATUS_CODE_MASK) ? 1u : 0u)


/* NVM Indext size */
#define VAL_NVM_BLOCK_SIZE       4u
#define VAL_NVM_OFFSET(nvm_idx)  ((uint32_t)(nvm_idx) * (uint32_t)VAL_NVM_BLOCK_SIZE)

typedef enum {
    NVM_PLATFORM_RESERVE_INDEX  =  0x0,
    NVM_CUR_SUITE_NUM_INDEX     =  0x1,
    NVM_CUR_TEST_NUM_INDEX      =  0x2,
    NVM_END_TEST_NUM_INDEX      =  0x3,
    NVM_TEST_PROGRESS_INDEX     =  0x4,
    NVM_TOTAL_PASS_INDEX        =  0x5,
    NVM_TOTAL_FAIL_INDEX        =  0x6,
    NVM_TOTAL_SKIP_INDEX        =  0x7,
    NVM_TOTAL_ERROR_INDEX       =  0x8,
    NVM_BOOT                    =  0x9,
    NVM_PREVIOUS_TEST_ID        =  0xA,
    NVM_TEST_DATA1              =  0xB,
    NVM_TEST_DATA2              =  0xC,
    NVM_TEST_DATA3              =  0xD,
} nvm_map_index_t;

#endif /* VAL_COMMON_H */
