#include "libmcu/cli.h"
#include "uart0.h"

struct cli_io const *cli_io_create(void)
{
	static bool initialized;

	if (!initialized) {
		uart0_init(115200);
	}

	static const struct cli_io io = {
		.read = uart0_read,
		.write = uart0_write_async,
	};

	return &io;
}
