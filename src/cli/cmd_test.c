/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"

cli_cmd_error_t cli_cmd_test(int argc, const char *argv[], const void *env)
{
	struct cli const *cli = (struct cli const *)env;

	cli->io->write("test\r\n", 6);

	return CLI_CMD_SUCCESS;
}
