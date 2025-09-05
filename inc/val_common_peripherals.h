/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef VAL_COMMON_PERIPHERALS_H
#define VAL_COMMON_PERIPHERALS_H

#include "val_common.h"

/* Use size_t for both offset and size to avoid width mismatch */
uint32_t val_nvm_read(size_t offset, void *buffer, size_t size);
uint32_t val_nvm_write(size_t offset, const void *buffer, size_t size);
uint32_t val_watchdog_enable(void);
uint32_t val_watchdog_disable(void);

#endif /* VAL_COMMON_PERIPHERALS_H */
