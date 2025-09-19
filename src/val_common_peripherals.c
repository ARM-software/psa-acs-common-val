/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val_common_peripherals.h"
#include <stdatomic.h>
#include <stdbool.h>

/*
 * Spin lock guarding watchdog operations to prevent concurrent reconfiguration.
 * These APIs can be invoked from different execution contexts (threads/interrupts) so
 * a lightweight atomic flag keeps the serialization overhead minimal without pulling
 * in platform specific primitives.
 */
static atomic_flag watchdog_lock = ATOMIC_FLAG_INIT;
static atomic_bool watchdog_is_enabled = ATOMIC_VAR_INIT(false);

static void val_watchdog_lock(void)
{
    while (atomic_flag_test_and_set_explicit(&watchdog_lock, memory_order_acquire)) {
        /* Busy-wait; watchdog operations are rare and quick. */
    }
}

static void val_watchdog_unlock(void)
{
    atomic_flag_clear_explicit(&watchdog_lock, memory_order_release);
}

/**
 *   @brief   -  Reads 'size' bytes from non-volatile memory 'base + offset' into given buffer
 *   @param   -  offset  : Offset from NV Memory base address
 *            -  buffer  : Pointer to destination address
 *            -  size    : Number of bytes
 *   @return  -  SUCCESS/FAILURE
**/
uint32_t val_nvm_read(uint32_t offset, void *buffer, size_t size)
{
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
      return pal_nvm_write(offset, buffer, size);
}

/**
 *   @brief   -  Initializes and enable the hardware Watchdog timer
 *   @param   -  void
 *   @return  -  SUCCESS/FAILURE
 **/
uint32_t val_watchdog_enable(void)
{
      uint32_t status;

      val_watchdog_lock();

      if (atomic_load_explicit(&watchdog_is_enabled, memory_order_relaxed)) {
          status = VAL_SUCCESS;
      } else {
          status = pal_watchdog_enable();
          if (status == VAL_SUCCESS) {
              atomic_store_explicit(&watchdog_is_enabled, true, memory_order_release);
          }
      }

      val_watchdog_unlock();

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

      val_watchdog_lock();

      if (!atomic_load_explicit(&watchdog_is_enabled, memory_order_relaxed)) {
          status = VAL_SUCCESS;
      } else {
          status = pal_watchdog_disable();
          if (status == VAL_SUCCESS) {
              atomic_store_explicit(&watchdog_is_enabled, false, memory_order_release);
          }
      }

      val_watchdog_unlock();

      return status;
}
