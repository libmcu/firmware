#include "drivers/bq25180.h"
#include "libmcu/compiler.h"

#if !defined(MIN)
#define MIN(a, b)		(((a) > (b))? (b) : (a))
#endif

#define MIN_BAT_REG_mV		3500
#define MAX_BAT_REG_mV		4650

#define MIN_IN_CURR_mA		5
#define MAX_IN_CURR_mA		1000

/*
7-bit addressing only
address: 0x6A (0xD4)
*/

enum mode {
	BATTERY_MODE,
	SHIP_MODE,
	CHARGE_ADAPTER_MODE,
	SHUTDOWN_MODE,
};

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

void bq25180_reset(bool hardware_reset)
{
	//SHIP_RST.REG_RST
	//SHIP_RST.EN_RST_SHIP
}

bool bq25180_read_event(struct bq25180_event *p)
{
	//FLAG0
	assert(p != NULL);
	return false;
}

bool bq25180_read_state(struct bq25180_state *p)
{
	//STAT0, STAT1
	assert(p != NULL);
	return false;
}

void bq25180_enable_battery_charging(bool enable);
{
	// ICHG_CTRL.ICHG_DIS to 1 for enable, 0 for disable
}

void bq25180_set_safety_timer(enum bq25180_safety_timer opt)
{
	//IC_CTRL.SAFETY_TIMER
	//IC_CTRL.2XTMR_EN
}

void bq25180_set_watchdog_timer(enum bq25180_watchdog opt)
{
	//IC_CTRL.WATCHDOG_SEL
	//SYS_REG.WATCHDOG_15S_ENABLE
}

void bq25180_set_battery_regulation_voltage(uint16_t millivoltage)
{
	assert(millivoltage >= MIN_BAT_REG_mV &&
			millivoltage <= MAX_BAT_REG_mV);
	uint8_t reg = (uint8_t)((millivoltage - MIN_BAT_REG_mV) / 10);

	unused(reg);
}

void bq25180_set_battery_discharge_current(
		enum bq25180_bat_discharge_current mA)
{
	//CHARGECTRL1.IBAT_OCP
}

void bq25180_set_battery_under_voltage(uint16_t millivoltage)
{
	// BCHARGECTRL1.UVLO
}

void bq25180_set_precharge_threshold(uint16_t millivoltage)
{
	uint8_t val = 0;

	if (millivoltage <= 2800) {
		val = 1;
	}

	// IC_CTRL.VLOWV_SEL
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

	// ICHG_CTRL.ITERM
}

void bq25180_enable_vindpm(bool enable)
{
	// CHARGECTRL0.VINDPM
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

	// TMR_ILIM.ILIM
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
