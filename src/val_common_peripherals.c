/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_common_peripherals.h"
#include "val_policy.h"

/**
 *   @brief   -  Reads 'size' bytes from non-volatile memory 'base + offset' into given buffer
 *   @param   -  offset  : Offset from NV Memory base address
 *            -  buffer  : Pointer to destination address
 *            -  size    : Number of bytes
 *   @return  -  SUCCESS/FAILURE
**/
uint32_t val_nvm_read(uint32_t offset, void *buffer, size_t size)
{
      /* Basic parameter validation */
      if ((buffer == NULL && size != 0)) {
          return VAL_STATUS_INVALID;
      }

      /* Prevent overflow in (offset + size) and enforce policy bounds.
       * Use 64-bit arithmetic to avoid truncation during comparison.
       */
      const uint64_t end64 = (uint64_t)offset + (uint64_t)size;
      const uint64_t max64 = (uint64_t)VAL_POLICY_NVM_MAX_SIZE;
      if (end64 > max64) {
          return VAL_STATUS_INVALID;
      }

      return pal_nvm_read(offset, buffer, size);
}

/**
 *    @brief   -  Writes 'size' bytes from buffer into non-volatile memory at a given
 *                'base + offset'
 *    @param   -  offset  : Offset from NV Memory base address
 *             -  buffer  : Pointer to source address
 *             -  size    : Number of bytes
 *    @return  -  SUCCESS/FAILURE
**/
uint32_t val_nvm_write(uint32_t offset, void *buffer, size_t size)
{
      /* Basic parameter validation */
      if ((buffer == NULL && size != 0)) {
          return VAL_STATUS_INVALID;
      }

      /* Prevent overflow in (offset + size) and enforce policy bounds.
       * Use 64-bit arithmetic to avoid truncation during comparison.
       */
      const uint64_t end64 = (uint64_t)offset + (uint64_t)size;
      const uint64_t max64 = (uint64_t)VAL_POLICY_NVM_MAX_SIZE;
      if (end64 > max64) {
          return VAL_STATUS_INVALID;
      }

      return pal_nvm_write(offset, buffer, size);
}

/**
 *   @brief   -  Initializes and enable the hardware Watchdog timer
 *   @param   -  void
 *   @return  -  SUCCESS/FAILURE
 **/
uint32_t val_watchdog_enable(void)
{
      return pal_watchdog_enable();
}

/**
 *   @brief   -  Disables the hardware Watchdog timer
 *   @param   -  void
 *   @return  -  SUCCESS/FAILURE
 **/
uint32_t val_watchdog_disable(void)
{
      return pal_watchdog_disable();
}
