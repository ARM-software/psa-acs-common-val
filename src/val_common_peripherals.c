/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_common_peripherals.h"
#include <stdint.h>
#include <stddef.h>

/**
 *   @brief   -  Reads 'size' bytes from non-volatile memory 'base + offset' into given buffer
 *   @param   -  offset  : Offset from NV Memory base address
 *            -  buffer  : Pointer to destination address
 *            -  size    : Number of bytes
 *   @return  -  SUCCESS/FAILURE
**/
uint32_t val_nvm_read(uint32_t offset, void *buffer, size_t size)
{
      /* Accept zero-size operations as a no-op even with NULL buffer */
      if (size == 0) {
            return 0; /* no-op success */
      }

      /* Basic parameter validation to avoid OOB or faults */
      if (buffer == NULL) {
            return 1; /* invalid parameter */
      }

      /* prevent 32-bit overflow on offset + size */
      if (size > (size_t)(UINT32_MAX - offset)) {
            return 1; /* invalid range */
      }

      /* Optional: enforce total NVM bounds if platform provides size */
#if defined(VAL_NVM_TOTAL_SIZE)
      if ((uint64_t)offset + (uint64_t)size > (uint64_t)VAL_NVM_TOTAL_SIZE) {
            return 1; /* out of NVM bounds */
      }
#endif

      /* Optional: enforce per-operation maximum if provided */
#if defined(VAL_NVM_MAX_RW_SIZE)
      if (size > (size_t)VAL_NVM_MAX_RW_SIZE) {
            return 1; /* request too large */
      }
#endif

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
      /* Accept zero-size operations as a no-op even with NULL buffer */
      if (size == 0) {
            return 0; /* no-op success */
      }

      /* Basic parameter validation to avoid OOB or faults */
      if (buffer == NULL) {
            return 1; /* invalid parameter */
      }

      /* prevent 32-bit overflow on offset + size */
      if (size > (size_t)(UINT32_MAX - offset)) {
            return 1; /* invalid range */
      }

      /* Optional: enforce total NVM bounds if platform provides size */
#if defined(VAL_NVM_TOTAL_SIZE)
      if ((uint64_t)offset + (uint64_t)size > (uint64_t)VAL_NVM_TOTAL_SIZE) {
            return 1; /* out of NVM bounds */
      }
#endif

      /* Optional: enforce per-operation maximum if provided */
#if defined(VAL_NVM_MAX_RW_SIZE)
      if (size > (size_t)VAL_NVM_MAX_RW_SIZE) {
            return 1; /* request too large */
      }
#endif

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
