#include "drivers/bq25180.h"
#include "libmcu/compiler.h"

int bq25180_disable_battery_charging(bool enable)
{
	// ICHG_CTRL.ICHG_DIS to 1 for enable, 0 for disable
	return 0;
}

int bq25180_enable_vindpm(bool enable)
{
	// CHARGECTRL0.VINDPM
	return 0;
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
