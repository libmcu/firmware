/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "bq25180_overrides.h"
#include "i2c0.h"

static struct i2c *i2c_handle;

static void initialize_i2c(void)
{
	i2c_handle = i2c0_create();
	i2c_init(i2c_handle);
}

int bq25180_read(uint8_t addr, uint8_t reg, void *buf, size_t bufsize)
{
	if (!i2c_handle) {
		initialize_i2c();
	}
	return i2c_read(i2c_handle, addr, reg, buf, bufsize);
}

int bq25180_write(uint8_t addr, uint8_t reg, const void *data, size_t data_len)
{
	if (!i2c_handle) {
		initialize_i2c();
	}
	return i2c_write(i2c_handle, addr, reg, data, data_len);
}
