/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_common_peripherals.h"

/**
 *   @brief   -  Reads 'size' bytes from non-volatile memory 'base + offset' into given buffer
 *   @param   -  offset  : Offset from NV Memory base address
 *            -  buffer  : Pointer to destination address
 *            -  size    : Number of bytes
 *   @return  -  SUCCESS/FAILURE
 *
 *   Validates parameters to avoid integer overflow in (offset + size)
 *   computation and null buffer dereference when size > 0.
**/
uint32_t val_nvm_read(uint32_t offset, void *buffer, size_t size)
{
      /* Zero-length read is a no-op */
      if (size == 0u) {
          return VAL_SUCCESS;
      }

      /* Buffer must be valid for non-zero size */
      if (buffer == NULL) {
          return VAL_STATUS_INVALID;
      }

      /* Prevent 32-bit overflow in (offset + size) calculation */
      if (size > (size_t)(UINT32_MAX - offset)) {
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
 *
 *    Validates parameters to avoid integer overflow in (offset + size)
 *    computation and null buffer dereference when size > 0.
**/
uint32_t val_nvm_write(uint32_t offset, void *buffer, size_t size)
{
      /* Zero-length write is a no-op */
      if (size == 0u) {
          return VAL_SUCCESS;
      }

      /* Buffer must be valid for non-zero size */
      if (buffer == NULL) {
          return VAL_STATUS_INVALID;
      }

      /* Prevent 32-bit overflow in (offset + size) calculation */
      if (size > (size_t)(UINT32_MAX - offset)) {
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
