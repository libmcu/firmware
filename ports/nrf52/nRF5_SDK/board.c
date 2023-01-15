/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "libmcu/board.h"
#include "libmcu/assert.h"
#include "nrf_pwr_mgmt.h"

void board_init(void)
{
	int rc = nrf_pwr_mgmt_init();
	assert(rc == NRF_SUCCESS);
}

void board_reboot(void)
{
	NVIC_SystemReset();
}
