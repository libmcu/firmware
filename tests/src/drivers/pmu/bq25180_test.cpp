/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "drivers/bq25180.h"
#include "libmcu/assert.h"

int bq25180_read(uint8_t addr, uint8_t reg, void *buf, size_t bufsize) {
	return mock().actualCall(__func__)
			.withParameter("addr", addr)
			.withParameter("reg", reg)
			.withOutputParameter("buf", buf)
			.withParameter("bufsize", bufsize)
			.returnIntValueOrDefault((int)bufsize);
}

int bq25180_write(uint8_t addr, uint8_t reg, const void *data, size_t data_len) {
	return mock().actualCall(__func__)
			.withParameter("addr", addr)
			.withParameter("reg", reg)
			.withMemoryBufferParameter("data", (const uint8_t *)data, data_len)
			.returnIntValueOrDefault((int)data_len);
}

void libmcu_assertion_failed(const uintptr_t *pc, const uintptr_t *lr) {
	mock().actualCall(__func__);
	TEST_EXIT;
}

TEST_GROUP(BQ25180) {
	void setup(void) {
		mock().strictOrder();
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}

	void expect_reg_read(uint8_t reg, uint8_t *p) {
		mock().expectOneCall("bq25180_read")
			.withParameter("addr", BQ25180_DEVICE_ADDRESS)
			.withParameter("reg", reg)
			.withOutputParameterReturning("buf", p, sizeof(*p))
			.withParameter("bufsize", 1);
	}
	void expect_reg_write(uint8_t reg, uint8_t *p) {
		mock().expectOneCall("bq25180_write")
			.withParameter("addr", BQ25180_DEVICE_ADDRESS)
			.withParameter("reg", reg)
			.withMemoryBufferParameter("data", p, sizeof(*p));
	}
};

TEST(BQ25180, reset_ShouldDoSoftReset_WhenHardwareResetFlagIsFalse) {
	uint8_t reset_val = 0x11;
	uint8_t expected = 0x91;

	expect_reg_read(0x09, &reset_val);
	expect_reg_write(0x09, &expected);

	bq25180_reset(0);
}

TEST(BQ25180, reset_ShouldDoHardwareReset_WhenHardwareResetFlagIsTrue) {
	uint8_t reset_val = 0x11;
	uint8_t expected = 0x71;

	expect_reg_read(0x09, &reset_val);
	expect_reg_write(0x09, &expected);

	bq25180_reset(true);
}

TEST(BQ25180, read_event_ShouldReturnEvents_WhenNoEventOccured) {
	uint8_t default_flag0 = 0;
	struct bq25180_event expected = { 0, };
	struct bq25180_event actual;

	expect_reg_read(0x02, &default_flag0);

	LONGS_EQUAL(true, bq25180_read_event(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_event_ShouldReturnEvents_WhenAllEventsOccured) {
	uint8_t flag0 = 0xff;
	struct bq25180_event expected = {
		.battery_overcurrent = 1,
		.battery_undervoltage = 1,
		.input_overvoltage = 1,
		.thermal_regulation = 1,
		.vindpm_fault = 1,
		.vdppm_fault = 1,
		.ilim_fault = 1,
		.battery_thermal_fault = 1,
	};
	struct bq25180_event actual;

	expect_reg_read(0x02, &flag0);

	LONGS_EQUAL(true, bq25180_read_event(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_event_ShouldReturnEvents_WhenSomeEventsOccured) {
	uint8_t flag0 = 0x55;
	struct bq25180_event expected = {
		.battery_overcurrent = 1,
		.battery_undervoltage = 0,
		.input_overvoltage = 1,
		.thermal_regulation = 0,
		.vindpm_fault = 1,
		.vdppm_fault = 0,
		.ilim_fault = 1,
		.battery_thermal_fault = 0,
	};
	struct bq25180_event actual;

	expect_reg_read(0x02, &flag0);

	LONGS_EQUAL(true, bq25180_read_event(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_event_ShouldAssertParam_WhenNullParamGiven) {
	mock().expectOneCall("libmcu_assertion_failed");
	bq25180_read_event(NULL);
}

TEST(BQ25180, read_state_ShouldReturnState) {
	uint8_t stat0 = 0;
	uint8_t stat1 = 0;
	struct bq25180_state expected = { 0, };
	struct bq25180_state actual;

	expect_reg_read(0x00, &stat0);
	expect_reg_read(0x01, &stat1);

	LONGS_EQUAL(true, bq25180_read_state(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_state_ShouldReturnState_WhenAllStateAreSetToOnes) {
	uint8_t stat0 = 0xff;
	uint8_t stat1 = 0xff;
	struct bq25180_state expected = {
		.vin_good = 1,
		.thermal_regulation_active = 1,
		.vindpm_active = 1,
		.vdppm_active = 1,
		.ilim_active = 1,
		.charging_status = 3,
		.tsmr_open = 1,
		.wake2_raised = 1,
		.wake1_raised = 1,
		.safety_timer_fault = 1,
		.ts_status = 3,
		.battery_undervoltage_active = 1,
		.vin_overvoltage_active = 1,
	};

	struct bq25180_state actual;

	expect_reg_read(0x00, &stat0);
	expect_reg_read(0x01, &stat1);

	LONGS_EQUAL(true, bq25180_read_state(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_state_ShouldReturnState_WhenSomeStateAreSetToOnes) {
	uint8_t stat0 = 0x35;
	uint8_t stat1 = 0x95;
	struct bq25180_state expected = {
		.vin_good = 1,
		.thermal_regulation_active = 0,
		.vindpm_active = 1,
		.vdppm_active = 0,
		.ilim_active = 1,
		.charging_status = 1,
		.tsmr_open = 0,
		.wake2_raised = 1,
		.wake1_raised = 0,
		.safety_timer_fault = 1,
		.ts_status = 2,
		.battery_undervoltage_active = 0,
		.vin_overvoltage_active = 1,
	};

	struct bq25180_state actual;

	expect_reg_read(0x00, &stat0);
	expect_reg_read(0x01, &stat1);

	LONGS_EQUAL(true, bq25180_read_state(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_state_ShouldAssertParam_WhenNullParamGiven) {
	mock().expectOneCall("libmcu_assertion_failed");
	bq25180_read_state(NULL);
}

TEST(BQ25180, battery_charging_ShouldDisable) {
	uint8_t reset_val = 0x05;
	uint8_t expected = 0x85;

	expect_reg_read(0x04/*ICHG_CTRL*/, &reset_val);
	expect_reg_write(0x04, &expected);

	bq25180_enable_battery_charging(false);
}

TEST(BQ25180, battery_charging_ShouldEnable) {
	uint8_t val = 0x85;
	uint8_t expected = 0x05;

	expect_reg_read(0x04/*ICHG_CTRL*/, &val);
	expect_reg_write(0x04, &expected);

	bq25180_enable_battery_charging(true);
}

TEST(BQ25180, safety_timer_ShouldBeSet_When3HGiven) {
	uint8_t reset_val = 0x84;
	uint8_t expected = 0x80;

	expect_reg_read(0x07/*IC_CTRL*/, &reset_val);
	expect_reg_write(0x07, &expected);

	bq25180_set_safety_timer(BQ25180_SAFETY_3H);
}

TEST(BQ25180, safety_timer_ShouldBeDisabled_WhenRequested) {
	uint8_t reset_val = 0x84;
	uint8_t expected = 0x8C;

	expect_reg_read(0x07/*IC_CTRL*/, &reset_val);
	expect_reg_write(0x07, &expected);

	bq25180_set_safety_timer(BQ25180_SAFETY_DISABLE);
}

TEST(BQ25180, watchdog_timer_ShouldBeSet_When40sGiven) {
	uint8_t reset_val = 0x84;
	uint8_t expected = 0x86;

	expect_reg_read(0x07/*IC_CTRL*/, &reset_val);
	expect_reg_write(0x07, &expected);

	bq25180_set_watchdog_timer(BQ25180_WDT_40_SEC);
}

TEST(BQ25180, watchdog_timer_ShouldBeDisabled_WhenRequested) {
	uint8_t reset_val = 0x84;
	uint8_t expected = 0x87;

	expect_reg_read(0x07/*IC_CTRL*/, &reset_val);
	expect_reg_write(0x07, &expected);

	bq25180_set_watchdog_timer(BQ25180_WDT_DISABLE);
}

TEST(BQ25180, battery_regulation_ShouldSetVoltage_WhenMilliVoltageGiven) {
	uint8_t expected = 0x46;
	expect_reg_write(0x03/*VBAT_CTRL*/, &expected);
	bq25180_set_battery_regulation_voltage(4200);
	expected = 0x00, expect_reg_write(0x03/*VBAT_CTRL*/, &expected);
	bq25180_set_battery_regulation_voltage(3500);
	expected = 0x73, expect_reg_write(0x03/*VBAT_CTRL*/, &expected);
	bq25180_set_battery_regulation_voltage(4650);
}

TEST(BQ25180, battery_regulation_ShouldAssertParam_WhenBelowTheRange) {
	mock().expectOneCall("libmcu_assertion_failed");
	bq25180_set_battery_regulation_voltage(3500-1);
}

TEST(BQ25180, battery_regulation_ShouldAssertParam_WhenAboveTheRange) {
	mock().expectOneCall("libmcu_assertion_failed");
	bq25180_set_battery_regulation_voltage(4650+1);
}
