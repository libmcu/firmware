/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cli.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "libmcu/compiler.h"
#include "libmcu/trace.h"

static void println(const struct cli_io *io, const char *str)
{
	io->write(str, strlen(str));
	io->write("\n", 1);
}

LIBMCU_NO_INSTRUMENT
static void trace_callback(const struct trace *entry, void *ctx)
{
	char buf[34] = { 0, };
	const struct cli_io *io = (const struct cli_io *)ctx;

	snprintf(buf, sizeof(buf), "%12" PRIu32 " %8" PRIxPTR " %5zu ",
			entry->timestamp, (uintptr_t)entry->thread,
			entry->stack_usage);
	io->write(buf, strlen(buf));

	for (uint8_t i = 0; i < entry->depth; i++) {
		io->write("  ", 2);
	}

	snprintf(buf, sizeof(buf), "%p (from %p)",
			entry->callee, entry->caller);
	io->write(buf, strlen(buf));
	io->write("\n", 1);
}

static void print_trace(const struct cli_io *io)
{
	size_t n;

	if ((n = trace_count()) == 0) {
		return;
	}

	union {
		const struct cli_io *cio;
		void *p;
	} t = { .cio = io, };

	println(io, " timestamp    thread  stack function");

	trace_iterate(trace_callback, t.p, -1);
	trace_clear();

	char buf[16] = { 0, };
	snprintf(buf, sizeof(buf), "%zu / %u", n, TRACE_MAXLEN);
	println(io, buf);
}

static cli_cmd_error_t handle_single_param(const char *argv[],
					   const struct cli_io *io)
{
	if (strcmp(argv[1], "help") == 0) {
		println(io, "subcommands: trace");
	} else if (strcmp(argv[1], "trace") == 0) {
		print_trace(io);
	}

	return CLI_CMD_SUCCESS;
}

DEFINE_CLI_CMD(dbg, "Debug functions such as trace") {
	struct cli const *cli = (struct cli const *)env;

	if (argc == 2) {
		return handle_single_param(argv, cli->io);
	}

	return CLI_CMD_SUCCESS;
}
