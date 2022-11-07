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

#define BQ25180_DEVICE_ADDRESS		0x6A /* 7-bit addressing only */

enum bq25180_sys_source {
	BQ25180_SYS_SRC_VIN_VBAT, /**< Powered from VIN if present or VBAT */
	BQ25180_SYS_SRC_VBAT, /**< Powered from VBAT only, even if VIN present */
	BQ25180_SYS_SRC_NONE_FLOATING, /**< Disconnected and left floating */
	BQ25180_SYS_SRC_NONE_PULLDOWN, /**< Disconnected with pulldown */
};

enum bq25180_sys_regulation {
	BQ25180_SYS_REG_VBAT, /**< VBAT + 225 mV (3.8 V minimum) */
	BQ25180_SYS_REG_V4_4, /**< 4.4V */
	BQ25180_SYS_REG_V4_5, /**< 4.5V */
	BQ25180_SYS_REG_V4_6, /**< 4.6V */
	BQ25180_SYS_REG_V4_7, /**< 4.7V */
	BQ25180_SYS_REG_V4_8, /**< 4.8V */
	BQ25180_SYS_REG_V4_9, /**< 4.9V */
	BQ25180_SYS_REG_PASS_THROUGH, /**< Pass through */
};

enum bq25180_bat_discharge_current {
	BQ25180_BAT_DISCHAGE_500mA,
	BQ25180_BAT_DISCHAGE_1000mA,
	BQ25180_BAT_DISCHAGE_1500mA,
	BQ25180_BAT_DISCHAGE_DISABLE,
};

enum bq25180_safety_timer {
	BQ25180_SAFETY_3H, /**< 3 hour fast charge */
	BQ25180_SAFETY_6H, /**< 6 hour fast charge */
	BQ25180_SAFETY_12H, /**< 12 hour fast charge */
	BQ25180_SAFETY_DISABLE, /**< disable safty timer */
};

enum bq25180_watchdog {
	BQ25180_WDT_DEFAULT, /**< 160s hardware reset */
	BQ25180_WDT_160_SEC, /**< 160s hardware reset */
	BQ25180_WDT_40_SEC, /**< 40s hardware reset */
	BQ25180_WDT_DISABLE, /**< Disable watchdog function */
};

struct bq25180_event {
	uint8_t battery_overcurrent  : 1;
	uint8_t battery_undervoltage : 1;
	uint8_t input_overvoltage    : 1;
	uint8_t thermal_regulation   : 1;
	uint8_t vindpm_fault         : 1;
	uint8_t vdppm_fault          : 1;
	uint8_t ilim_fault           : 1;
	uint8_t battery_thermal_fault: 1;
};

struct bq25180_state {
	uint16_t vin_good                   : 1;
	uint16_t thermal_regulation_active  : 1;
	uint16_t vindpm_active              : 1;
	uint16_t vdppm_active               : 1;
	uint16_t ilim_active                : 1;
	uint16_t charging_status            : 2;
	uint16_t tsmr_open                  : 1;
	uint16_t wake2_raised               : 1;
	uint16_t wake1_raised               : 1;
	uint16_t safety_timer_fault         : 1;
	uint16_t ts_status                  : 2;
	uint16_t battery_undervoltage_active: 1;
	uint16_t vin_overvoltage_active     : 1;
};

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
 * @brief Read the device events
 *
 * @param[in] p @ref bq25180_event
 *
 * @return true on success or false
 */
bool bq25180_read_event(struct bq25180_event *p);

/**
 * @brief Read the device state
 *
 * @param[in] p @ref bq25180_state
 *
 * @return true on success or false
 */
bool bq25180_read_state(struct bq25180_state *p);

/**
 * @brief Enable or disable battery charging
 *
 * @param[in] enable battery charging to be enabled if true or false to be
 *                   disabled
 */
void bq25180_enable_battery_charging(bool enable);

/**
 * @brief Set the safety time
 *
 * If charging has not terminated before the programmed safety time, charging
 * is disabled.
 *
 * When the safety timer is active, changing the safety timer duration resets
 * the safety timer.
 *
 * 6 hour by default on reset.
 *
 * @param[in] opt one of @ref bq25180_safety_timer
 */
void bq25180_set_safety_timer(enum bq25180_safety_timer opt);

/**
 * @brief Set the watchdog time
 *
 * Once the initial transaction is received, the watchdog timer is started. The
 * watchdog timer is reset by any transaction by the host using the I2C
 * interface. If the watchdog timer expires without a reset from the I2C
 * interface, all charger parameters registers are reset to the default values.
 *
 * 160 sec by default on reset.
 *
 * @param[in] opt one of @ref bq25180_watchdog
 */
void bq25180_set_watchdog_timer(enum bq25180_watchdog opt);

/**
 * @brief Set the battery regulation target
 *
 * The charging current starts tapering off once the battery voltage approches
 * the battery regulation target.
 *
 * 4200mV by default on reset.
 * 
 * @param[in] millivoltage from 3500mV up to 4650mV
 */
void bq25180_set_battery_regulation_voltage(uint16_t millivoltage);

/**
 * @brief Set the battery discharge current limit
 *
 * @ref BQ25180_BAT_DISCHAGE_500mA by default on reset.
 *
 * @param[in] mA one of @ref bq25180_bat_discharge_current
 */
void bq25180_set_battery_discharge_current(
		enum bq25180_bat_discharge_current mA);

/**
 * @brief Set battery undervoltage lockout falling threshold
 *
 * When voltage at the battery drops below the configured, the BAT and SYS path
 * gets disengaged.
 *
 * The below levels are supported:
 * - 3000mV
 * - 2800mV
 * - 2600mV
 * - 2400mV
 * - 2200mV
 * - 2000mV
 *
 * 3000mV by default on reset.
 * 
 * @param[in] millivoltage from 2000mV up to 3000mV
 */
void bq25180_set_battery_under_voltage(uint16_t millivoltage);

/**
 * @brief Set the precharge voltage threshold
 *
 * 3000mV by default on reset.
 *
 * @param[in] millivoltage either of 2800mV or 3000mV only
 */
void bq25180_set_precharge_threshold(uint16_t millivoltage);

/**
 * @brief Set the precharge current 
 *
 * The precharge current is the same to termination current by default on reset.
 *
 * @param[in] double_termination_current double of termination current if true
 *            or same to the termination current if false
 */
void bq25180_set_precharge_current(bool double_termination_current);

/**
 * @brief Set the maximum charge current level
 *
 * 10mA by default on reset.
 *
 * @param[in] milliampere from 5mA up to 1000mA
 */
void bq25180_set_fastcharge_current(uint16_t milliampere);

/**
 * @brief Set the termination current
 *
 * 10% by default on reset.
 *
 * @param[in] pct percentage of the maximum charge current level
 */
void bq25180_set_termination_current(uint8_t pct);

/**
 * @brief Enable of disable Input Voltage Based Dynamic Power Management
 *
 * This feature is enabled by default on reset.
 *
 * @param[in] enable true to enable, false to disable
 */
void bq25180_enable_vindpm(bool enable);

/**
 * @brief Enable of disable Dynamic Power Path Management Mode
 *
 * This feature is enabled by default on reset.
 *
 * @param[in] enable true to enable, false to disable
 */
void bq25180_enable_dppm(bool enable);

/**
 * @brief Set input voltage threshold to keep the input voltage higher
 *
 * @param[in] millivoltage 4200mV, 4500mV, 4700mV and 0 to disable are
 *            supported
 */
void bq25180_set_vindpm_voltage(uint16_t millivoltage);

/**
 * @brief Set the maximum input current
 *
 * The input DPM loop reduces input current if the sum of the charging and load
 * currents exceeds the preset maximum input current.
 *
 * 500mA by default on reset.
 *
 * @param[in] milliampere from 50mA up to 1100mA
 */
void bq25180_set_input_current(uint16_t milliampere);

/**
 * @brief Set regulated system voltage source
 *
 * This sets how SYS is powered in any state, except SHIPMODE.
 *
 * @ref BQ25180_SYS_SRC_VIN_VBAT by default on reset.
 *
 * @param[in] source one of @ref bq25180_sys_source
 */
void bq25180_set_sys_source(enum bq25180_sys_source source);

/**
 * @brief Set SYS regulation voltgage
 *
 * @ref BQ25180_SYS_REG_V4_4 by default on reset.
 *
 * @param[in] val one of @ref bq25180_sys_regulation
 */
void bq25180_set_sys_voltage(enum bq25180_sys_regulation val);

/**
 * @brief Read a register value via I2C
 *
 * @param[in] addr device address
 * @param[in] reg register address to read from
 * @param[in] buf buffer to get the value in
 * @param[in] bufsize size of @ref buf
 *
 * @return The number of bytes read on success. Otherwise a negative integer
 *         error code
 *
 * @note This function should be implemented for the specific platform or
 *       board. The default one does nothing being linked weak.
 */
int bq25180_read(uint8_t addr, uint8_t reg, void *buf, size_t bufsize);

/**
 * @brief Write a value to a register via I2C
 *
 * @param[in] addr device address
 * @param[in] reg register address to write to
 * @param[in] data data to be written in @ref reg
 * @param[in] data_len length of @ref data
 *
 * @return The number of bytes written on success. Otherwise a negative integer
 *         error code
 *
 * @note This function should be implemented for the specific platform or
 *       board. The default one does nothing being linked weak.
 */
int bq25180_write(uint8_t addr, uint8_t reg, const void *data, size_t data_len);

#if defined(__cplusplus)
}
#endif

#endif /* FPL_BQ25180_H */
