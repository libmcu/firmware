/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FPL_BQ25180_H
#define FPL_BQ25180_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Reset the system
 *
 * The device will reset all of the registers to the defaults. A hardware reset
 * to completely powercycle the system.
 *
 * @param[in] hardware_reset hardware reset if true while soft reset if false
 *
 * @note A hardware or software reset will cancel the pending shipmode request.
 */
void bq25180_reset(bool hardware_reset);

/**
 * @brief Enable or disable battery charging
 *
 * @param[in] enable battery charging to be enabled if true or to be disabled
 */
void bq25180_enable_battery_charging(bool enable);

/**
 * @brief Read a register value via I2C
 *
 * This function should be implemented for the specific platform or board. The
 * default one does nothing being linked weak.
 *
 * @param[in] addr device address
 * @param[in] reg register address to read from
 * @param[in] buf buffer to get the value in
 * @param[in] bufsize size of @ref buf
 *
 * @return The number of bytes read on success. Otherwise a negative integer
 *         error code
 */
int bq25180_read(uint8_t addr, uint8_t reg, void *buf, size_t bufsize);

/**
 * @brief Write a value to a register via I2C
 *
 * This function should be implemented for the specific platform or board. The
 * default one does nothing being linked weak.
 *
 * @param[in] addr device address
 * @param[in] reg register address to write to
 * @param[in] data data to be written in @ref reg
 * @param[in] data_len length of @ref data
 *
 * @return The number of bytes written on success. Otherwise a negative integer
 *         error code
 */
int bq25180_write(uint8_t addr, uint8_t reg, const void *data, size_t data_len);

#if defined(__cplusplus)
}
#endif

#endif /* FPL_BQ25180_H */
