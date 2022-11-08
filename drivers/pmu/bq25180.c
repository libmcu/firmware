/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "drivers/bq25180.h"

#include <string.h>

#include "libmcu/assert.h"
#include "libmcu/compiler.h"

#if !defined(MIN)
#define MIN(a, b)		(((a) > (b))? (b) : (a))
#endif

#define MIN_BAT_REG_mV		3500U
#define MAX_BAT_REG_mV		4650U

#define MIN_BAT_UNDERVOLTAGE_mV	2000U
#define MAX_BAT_UNDERVOLTAGE_mV	3000U

#define MIN_IN_CURR_mA		5U
#define MAX_IN_CURR_mA		1000U

enum registers {
	STAT0		= 0,	/* Charger Status */
	STAT1,			/* Charger Status and Faults */
	FLAG0,			/* Charger Flag Registers */
	VBAT_CTRL,		/* Battery Voltage Control */
	ICHG_CTRL,		/* Fast Charge Current Control */
	CHARGECTRL0,		/* Charger Control 0 */
	CHARGECTRL1,		/* Charger Control 1 */
	IC_CTRL,		/* IC Control */
	TMR_ILIM,		/* Timer and Input Current Limit Control */
	SHIP_RST,		/* Shipmode, Reset and Pushbutton Control */
	SYS_REG,		/* SYS Regulation Voltage Control */
	TS_CONTROL,		/* TS Control */
	MASK_ID,		/* MASK and Device ID */
};

static bool write_reg(uint8_t reg, uint8_t val)
{
	return bq25180_write(BQ25180_DEVICE_ADDRESS, reg, &val, 1) == 1;
}

static bool read_reg(uint8_t reg, uint8_t *p)
{
	return bq25180_read(BQ25180_DEVICE_ADDRESS, reg, p, 1) == 1;
}

void bq25180_reset(bool hardware_reset)
{
	uint8_t val;

	read_reg(SHIP_RST, &val);

	if (hardware_reset) {
		val |= 0x60U; /* EN_RST_SHIP */
	} else {
		val |= 0x80U; /* REG_RST */
	}

	write_reg(SHIP_RST, val);
}

bool bq25180_read_event(struct bq25180_event *p)
{
	uint8_t val;

	assert(p != NULL);

	if (!read_reg(FLAG0, &val)) {
		return false;
	}

	memset(p, 0, sizeof(*p));

	p->battery_overcurrent = val & 1U; /* BAT_OCP_FAULT */
	p->battery_undervoltage = (val >> 1) & 1U; /* BUVLO_FAULT_FLAG */
	p->input_overvoltage = (val >> 2) & 1U; /* VIN_OVP_FAULT_FLAG */
	p->thermal_regulation = (val >> 3) & 1U; /* THERMREG_ACTIVE_FLAG */
	p->vindpm_fault = (val >> 4) & 1U; /* VINDPM_ACTIVE_FLAG */
	p->vdppm_fault = (val >> 5) & 1U; /* VDPPM_ACTIVE_FLAG */
	p->ilim_fault = (val >> 6) & 1U; /* ILIM_ACTIVE_FLAG */
	p->battery_thermal_fault = (val >> 7) & 1U; /* TS_FAULT */

	return true;
}

bool bq25180_read_state(struct bq25180_state *p)
{
	uint8_t val0, val1;

	assert(p != NULL);

	if (!read_reg(STAT0, &val0) || !read_reg(STAT1, &val1)) {
		return false;
	}

	memset(p, 0, sizeof(*p));

	p->vin_good = val0 & 1U; /* VIN_PGOOD_STAT */
	p->thermal_regulation_active = (val0 >> 1) & 1U; /* THERMREG_ACTIVE_STAT */
	p->vindpm_active = (val0 >> 2) & 1U; /* VINDPM_ACTIVE_STAT */
	p->vdppm_active = (val0 >> 3) & 1U; /* VDPPM_ACTIVE_STAT */
	p->ilim_active = (val0 >> 4) & 1U; /* ILIM_ACTIVE_STAT */
	p->charging_status = (val0 >> 5) & 3U; /* CHG_STAT */
	p->tsmr_open = (val0 >> 7) & 1U; /* TS_OPEN_STAT */
	p->wake2_raised = val1 & 1U; /* WAKE2_FLAG */
	p->wake1_raised = (val1 >> 1) & 1U; /* WAKE1_FLAG */
	p->safety_timer_fault = (val1 >> 2) & 1U; /* SAFETY_TMR_FAULT_FLAG */
	p->ts_status = (val1 >> 3) & 3U; /* TS_STAT */
	p->battery_undervoltage_active = (val1 >> 6) & 1U; /* BUVLO_START */
	p->vin_overvoltage_active = (val1 >> 7) & 1U; /* VIN_OVP_STAT */

	return true;
}

void bq25180_enable_battery_charging(bool enable)
{
	uint8_t val;

	read_reg(ICHG_CTRL, &val);

	val = val & (uint8_t)~0x80U; /* CHG_DIS */

	if (!enable) {
		val = val | (uint8_t)0x80U;
	}

	write_reg(ICHG_CTRL, val);
}

void bq25180_set_safety_timer(enum bq25180_safety_timer opt)
{
	/* TODO: support IC_CTRL.2XTMR_EN */
	uint8_t val;

	read_reg(IC_CTRL, &val);

	val = val & (uint8_t)~0x0cu; /* SAFETY_TIMER */
	val = val | (uint8_t)(opt << 2);

	write_reg(IC_CTRL, val);
}

void bq25180_set_watchdog_timer(enum bq25180_watchdog opt)
{
	/* TODO: support SYS_REG.WATCHDOG_15S_ENABLE */
	uint8_t val;

	read_reg(IC_CTRL, &val);

	val = val & (uint8_t)~0x03U; /* WATCHDOG_SEL */
	val = val | (uint8_t)opt;

	write_reg(IC_CTRL, val);
}

void bq25180_set_battery_regulation_voltage(uint16_t millivoltage)
{
	assert(millivoltage >= MIN_BAT_REG_mV &&
			millivoltage <= MAX_BAT_REG_mV);

	uint8_t reg = (uint8_t)((millivoltage - MIN_BAT_REG_mV) / 10);
	write_reg(VBAT_CTRL, reg);
}

void bq25180_set_battery_discharge_current(
		enum bq25180_bat_discharge_current opt)
{
	uint8_t val;

	read_reg(CHARGECTRL1, &val);

	val = val & (uint8_t)~0xc0U; /* IBAT_OCP */
	val = val | (uint8_t)(opt << 6);

	write_reg(CHARGECTRL1, val);
}

void bq25180_set_battery_under_voltage(uint16_t millivoltage)
{
	uint8_t val, reg;

	assert(millivoltage >= MIN_BAT_UNDERVOLTAGE_mV &&
			millivoltage <= MAX_BAT_UNDERVOLTAGE_mV);

	if (millivoltage > 2800) {
		val = 2;
	} else if (millivoltage > 2600) {
		val = 3;
	} else if (millivoltage > 2400) {
		val = 4;
	} else if (millivoltage > 2200) {
		val = 5;
	} else if (millivoltage > 2000) {
		val = 6;
	} else {
		val = 7;
	}

	read_reg(CHARGECTRL1, &reg);

	reg = reg & (uint8_t)~0x38U; /* UVLO */
	reg = reg | (uint8_t)(val << 3);

	write_reg(CHARGECTRL1, reg);
}

void bq25180_set_precharge_threshold(uint16_t millivoltage)
{
	uint8_t val = 0;

	if (millivoltage <= 2800) {
		val = 1;
	}

	uint8_t reg;

	read_reg(IC_CTRL, &reg);

	reg = reg & (uint8_t)~0x40U; /* VLOWV_SEL */
	reg = reg | (uint8_t)(val << 6);

	write_reg(IC_CTRL, reg);
}

void bq25180_set_precharge_current(bool double_termination_current)
{
	// ICHARGECTRL0.PRECHG
}

void bq25180_set_fastcharge_current(uint16_t milliampere)
{
	assert(milliampere >= 5 && milliampere <= MAX_IN_CURR_mA);

	uint8_t reg = (uint8_t)(milliampere - MIN_IN_CURR_mA);

	if (milliampere > 35) {
		/* NOTE: 36mA to 39mA not in the range.
		 * See the datasheet: Table 8-13. */
		reg = MIN((uint8_t)(milliampere / 10 + 27), 31);
	}

	unused(reg);
	// ICHG_CTRL.ICHG
}

void bq25180_set_termination_current(uint8_t pct)
{
	uint8_t val = 0;

	if (pct >= 20) {
		val = 3;
	} else if (pct >= 10) {
		val = 2;
	} else if (pct >= 5) {
		val = 1;
	}

	unused(val);
	// ICHG_CTRL.ITERM
}

void bq25180_enable_vindpm(bool enable)
{
	// CHARGECTRL0.VINDPM
}

void bq25180_enable_dppm(bool enable)
{
}

void bq25180_set_vindpm_voltage(uint16_t millivoltage)
{
}

void bq25180_set_input_current(uint16_t milliampere)
{
	uint8_t val = 0;

	if (milliampere >= 1100) {
		val = 7;
	} else if (milliampere >= 700) {
		val = 6;
	} else if (milliampere >= 500) {
		val = 5;
	} else if (milliampere >= 400) {
		val = 4;
	} else if (milliampere >= 300) {
		val = 3;
	} else if (milliampere >= 200) {
		val = 2;
	} else if (milliampere >= 100) {
		val = 1;
	}

	unused(val);
	// TMR_ILIM.ILIM
}

void bq25180_set_sys_source(enum bq25180_sys_source source)
{
}

void bq25180_set_sys_voltage(enum bq25180_sys_regulation val)
{
}

LIBMCU_WEAK
int bq25180_read(uint8_t addr, uint8_t reg, void *buf, size_t bufsize)
{
	unused(addr);
	unused(reg);
	unused(buf);
	unused(bufsize);

	return 0;
}

LIBMCU_WEAK
int bq25180_write(uint8_t addr, uint8_t reg, const void *data, size_t data_len)
{
	unused(addr);
	unused(reg);
	unused(data);
	unused(data_len);

	return 0;
}
