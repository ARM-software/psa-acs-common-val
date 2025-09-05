/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef VAL_COMMON_PERIPHERALS_H
#define VAL_COMMON_PERIPHERALS_H

#include "val_common.h"

/*
 * Reads 'size' bytes from non-volatile memory at 'offset' into 'buffer'.
 * - buffer must be non-NULL
 * - size may be zero (noop)
 * - offset + size must not overflow 32-bit range
 */
uint32_t val_nvm_read(uint32_t offset, void *buffer, size_t size);

/*
 * Writes 'size' bytes from 'buffer' into non-volatile memory at 'offset'.
 * - buffer must be non-NULL
 * - size may be zero (noop)
 * - offset + size must not overflow 32-bit range
 */
uint32_t val_nvm_write(uint32_t offset, const void *buffer, size_t size);
uint32_t val_watchdog_enable(void);
uint32_t val_watchdog_disable(void);

#endif /* VAL_COMMON_PERIPHERALS_H */
