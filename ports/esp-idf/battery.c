/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "battery.h"
#include "adc1.h"
#include "driver/gpio.h"

#define MONITOR_ENABLE_GPIO_NUMBER		4
#define MONITOR_GPIO_NUMBER			7 /* ADC1_6 */
#define MONITOR_INTR_GPIO_NUMBER		14
#define ADC_CHANNEL				6

static struct adc *adc;
static void (*dispatch_callback)(void);

static void IRAM_ATTR on_interrupt(void *arg)
{
	if (dispatch_callback) {
		(*dispatch_callback)();
	}
}

static void initialize_monitor_gpio(void)
{
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = (1ULL << MONITOR_ENABLE_GPIO_NUMBER),
	};
	gpio_config(&io_conf);

	/* bq25180 interrupt pin */
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = (1ULL << MONITOR_INTR_GPIO_NUMBER);
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);
	gpio_isr_handler_add(MONITOR_INTR_GPIO_NUMBER, on_interrupt, 0);
}

int battery_enable_monitor(bool enable)
{
	gpio_set_level(MONITOR_ENABLE_GPIO_NUMBER, enable);
	return 0;
}

int battery_level_raw(int nr_sampling)
{
	if (!adc || nr_sampling < 0) {
		return 0;
	}

	int sum = 0;

	/* FIXME: The ADC returns incorrect values except the first one when
	 * reading more than once without delay. */
	adc_get_raw(adc, ADC_CHANNEL); /* first two samples have a little higher values */
	adc_get_raw(adc, ADC_CHANNEL);
	for (int i = 0; i < nr_sampling; i++) {
		sum += adc_get_raw(adc, ADC_CHANNEL);
	}

	return sum / nr_sampling;
}

int battery_raw_to_millivolts(int raw)
{
	if (!adc) {
		return 0;
	}

	return adc_raw_to_millivolts(adc, raw);
}

int battery_init(void (*on_event_callback)(void))
{
	adc = adc1_create();
	adc_enable(adc, true);
	adc_calibrate(adc);

	initialize_monitor_gpio();

	dispatch_callback = on_event_callback;

	return 0;
}
