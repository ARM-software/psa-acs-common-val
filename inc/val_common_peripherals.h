/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef VAL_COMMON_PERIPHERALS_H
#define VAL_COMMON_PERIPHERALS_H

#include "val_common.h"

uint32_t val_nvm_read(size_t offset, void *buffer, size_t size);
uint32_t val_nvm_write(size_t offset, void *buffer, size_t size);
/*
 * Watchdog control APIs serialize access internally, ensuring that concurrent enable
 * or disable requests are handled deterministically.
 */
uint32_t val_watchdog_enable(void);
uint32_t val_watchdog_disable(void);

#endif /* VAL_COMMON_PERIPHERALS_H */
