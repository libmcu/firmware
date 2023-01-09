/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LIBMCU_BATTERY_H
#define LIBMCU_BATTERY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

int battery_init(void (*on_event_callback)(void));
int battery_enable_monitor(bool enable);
int battery_level_raw(int nr_sampling);
int battery_raw_to_millivolts(int raw);
uint8_t battery_check_level(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BATTERY_H */
