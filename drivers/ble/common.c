#include "net/ble.h"
#include <string.h>
#include <errno.h>

void ble_adv_payload_clear(struct ble_adv_payload *buf)
{
	memset(buf, 0, sizeof(*buf));
}

int ble_adv_payload_add(struct ble_adv_payload *buf, uint8_t type,
			const void *data, uint8_t data_len)
{
	if ((buf->index + data_len + 2) > sizeof(buf->payload)) {
		return -ENOSPC;
	}

	uint8_t *p = &buf->payload[buf->index];
	buf->index += data_len + 1/*len*/ + 1/*type*/;

	p[0] = data_len + 1;
	p[1] = type;
	memcpy(&p[2], data, data_len);

	return 0;
}
