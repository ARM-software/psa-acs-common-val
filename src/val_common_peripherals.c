/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_common_peripherals.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Lightweight spin locks serialize access to PAL peripheral operations. The PAL drivers are
 * not guaranteed to provide concurrency control, so we guard the entry points here with
 * target-agnostic atomic flags instead of platform-specific primitives.
 */
static atomic_flag nvm_lock = ATOMIC_FLAG_INIT;
static atomic_flag watchdog_lock = ATOMIC_FLAG_INIT;
static atomic_bool watchdog_is_enabled = ATOMIC_VAR_INIT(false);

static void val_lock(atomic_flag *lock)
{
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        /* Busy-wait; operations are expected to complete quickly. */
    }
}

static void val_unlock(atomic_flag *lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}

static bool val_nvm_range_is_valid(size_t offset, size_t size)
{
#ifdef PLATFORM_NVM_SIZE
    const size_t nvm_size = (size_t)PLATFORM_NVM_SIZE;
#else
    const size_t nvm_size = SIZE_MAX;
#endif

    if (size > SIZE_MAX - offset) {
        return false;
    }

    if (offset > (size_t)UINT32_MAX) {
        return false;
    }

    if (size > (size_t)UINT32_MAX) {
        return false;
    }

    if (offset > ((size_t)UINT32_MAX - size)) {
        return false;
    }

    if (offset > nvm_size) {
        return false;
    }

    if (size > (nvm_size - offset)) {
        return false;
    }

    return true;
}

/**
 *   @brief   -  Reads 'size' bytes from non-volatile memory 'base + offset' into given buffer
 *   @param   -  offset  : Offset from NV Memory base address
 *            -  buffer  : Pointer to destination address
 *            -  size    : Number of bytes
 *   @return  -  SUCCESS/FAILURE
**/
uint32_t val_nvm_read(size_t offset, void *buffer, size_t size)
{
      uint32_t status;

      if ((buffer == NULL && size != 0U) || !val_nvm_range_is_valid(offset, size)) {
          return VAL_STATUS_INVALID;
      }

      if (size == 0U) {
          return VAL_SUCCESS;
      }

      val_lock(&nvm_lock);
      status = pal_nvm_read((uint32_t)offset, buffer, size);
      val_unlock(&nvm_lock);

      return status;
}

/**
 *    @brief   -  Writes 'size' bytes from buffer into non-volatile memory at a given
 *                'base + offset'
 *    @param   -  offset  : Offset from NV Memory base address
 *             -  buffer  : Pointer to source address
 *             -  size    : Number of bytes
 *    @return  -  SUCCESS/FAILURE
**/
uint32_t val_nvm_write(size_t offset, void *buffer, size_t size)
{
      uint32_t status;

      if ((buffer == NULL && size != 0U) || !val_nvm_range_is_valid(offset, size)) {
          return VAL_STATUS_INVALID;
      }

      if (size == 0U) {
          return VAL_SUCCESS;
      }

      val_lock(&nvm_lock);
      status = pal_nvm_write((uint32_t)offset, buffer, size);
      val_unlock(&nvm_lock);

      return status;
}

/**
 *   @brief   -  Initializes and enable the hardware Watchdog timer
 *   @param   -  void
 *   @return  -  SUCCESS/FAILURE
 **/
uint32_t val_watchdog_enable(void)
{
      uint32_t status;

      val_lock(&watchdog_lock);

      if (atomic_load_explicit(&watchdog_is_enabled, memory_order_relaxed)) {
          status = VAL_SUCCESS;
      } else {
          status = pal_watchdog_enable();
          if (status == VAL_SUCCESS) {
              atomic_store_explicit(&watchdog_is_enabled, true, memory_order_release);
          }
      }

      val_unlock(&watchdog_lock);

      return status;
}

/**
 *   @brief   -  Disables the hardware Watchdog timer
 *   @param   -  void
 *   @return  -  SUCCESS/FAILURE
 **/
uint32_t val_watchdog_disable(void)
{
      uint32_t status;

      val_lock(&watchdog_lock);

      if (!atomic_load_explicit(&watchdog_is_enabled, memory_order_relaxed)) {
          status = VAL_SUCCESS;
      } else {
          status = pal_watchdog_disable();
          if (status == VAL_SUCCESS) {
              atomic_store_explicit(&watchdog_is_enabled, false, memory_order_release);
          }
      }

      val_unlock(&watchdog_lock);

      return status;
}
