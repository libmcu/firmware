/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"
#include <stdio.h>
#include <string.h>
#include "common/wifi.h"
#include "libmcu/hexdump.h"

#if !defined(MIN)
#define MIN(x, y)			(((x) > (y))? (y) : (x))
#endif

static const struct cli_io *io;
static unsigned int scan_index;

static inline const char *stringify_band(enum wifi_frequency_band band)
{
	switch (band) {
	case WIFI_FREQ_2_4_GHZ:
		return "2.4GHz";
	case WIFI_FREQ_5_GHZ:
		return "5GHz";
	case WIFI_FREQ_6_GHZ:
		return "6GHz";
	default:
		return "Unknown";
	}
}

static inline const char *stringify_security(enum wifi_security sec)
{
	switch (sec) {
	case WIFI_SEC_TYPE_NONE:
		return "Open";
	case WIFI_SEC_TYPE_PSK:
		return "PSK";
	case WIFI_SEC_TYPE_PSK_SHA256:
		return "PSK_SHA256";
	case WIFI_SEC_TYPE_PSK_SAE:
		return "PSK_SAE";
	default:
		return "Unknown";
	}
}

static inline const char *stringify_mode(enum wifi_mode mode)
{
	switch (mode) {
	case WIFI_MODE_INFRA:
		return "Infra";
	case WIFI_MODE_ACCESS_POINT:
		return "AP";
	case WIFI_MODE_MESH:
		return "Mesh";
	default:
		return "Unknown";
	}
}

static inline const char *stringify_state(enum wifi_state state)
{
	switch (state) {
	case WIFI_STATE_DISABLED:
		return "Disabled";
	case WIFI_STATE_DISCONNECTING:
		return "Disconnecting";
	case WIFI_STATE_DISCONNECTED:
		return "Disconnected";
	case WIFI_STATE_INACTIVE:
		return "Inactive";
	case WIFI_STATE_SCANNING:
		return "Scanning";
	case WIFI_STATE_AUTHENTICATING:
		return "Authenticating";
	case WIFI_STATE_ASSOCIATING:
		return "Associating";
	case WIFI_STATE_ASSOCIATED:
		return "Associated";
	default:
		return "Unknown";
	}
}

static void print_scan_result(const struct wifi_scan_result *entry)
{
	if (entry == NULL) {
		return;
	}

	char buf[128] = { 0, };
	char mac[20] = { 0, };

	if (scan_index == 0) {
		snprintf(buf, sizeof(buf),
				"\r\n%-4s | %-32s | %-13s | %-4s | %-10s | %s\r\n",
				"No.", "SSID", "Chan (Band)", "RSSI", "Security", "BSSID");
		io->write(buf, strlen(buf));
	}

	scan_index++;

	hexdump(mac, sizeof(mac), entry->mac, entry->mac_len);
	snprintf(buf, sizeof(buf),
			"%-4d | %.*s%s | %-4u (%-6s) | %-4d | %-10s | %s\r\n",
			scan_index, entry->ssid_len, entry->ssid,
			&"                                "[entry->ssid_len],
			entry->channel, stringify_band(entry->band), entry->rssi,
			stringify_security(entry->security), mac);
	io->write(buf, strlen(buf));
}

static void event_callback(const wifi_iface_t iface,
			   enum wifi_event evt, const void *data)
{
	char buf[16];
	int len = snprintf(buf, sizeof(buf), "WIFI EVT: %x\r\n", evt);

	switch (evt) {
	case WIFI_EVT_SCAN_RESULT:
		print_scan_result((const struct wifi_scan_result *)data);
		break;
	case WIFI_EVT_SCAN_DONE:
	case WIFI_EVT_STARTED:
	case WIFI_EVT_DISCONNECTED:
	case WIFI_EVT_CONNECTED:
	default:
		io->write(buf, MIN((size_t)len, 0));
		break;
	}
}

static void print_wifi_info(const wifi_iface_t iface)
{
	const char *str = stringify_mode(iface->mode);
	size_t len = (size_t)strlen(str);
	io->write(str, len);
	io->write("\r\n", 2);
	str = stringify_state(iface->state);
	len = (size_t)strlen(str);
	io->write(str, len);
	io->write("\r\n", 2);
}

cli_cmd_error_t cli_cmd_wifi(int argc, const char *argv[], const void *env)
{
	static wifi_iface_t iface;
	struct cli const *cli = (struct cli const *)env;

	io = cli->io;

	if (iface == NULL) {
		iface = wifi_create();
		wifi_register_event_callback(iface, event_callback);
	}

	if (argc == 1) {
		print_wifi_info(iface);
	} else if (argc == 2 && strcmp("scan", argv[1]) == 0) {
		if (wifi_scan(iface) == 0) {
			scan_index = 0;
		}
	} else if (argc == 2 && strcmp("disconnect", argv[1]) == 0) {
		wifi_disconnect(iface);
	} else if (argc == 4 && strcmp("connect", argv[1]) == 0) {
		struct wifi_conf conf = {
			.ssid = (uint8_t *)argv[2],
			.ssid_len = strlen(argv[2]),
			.psk = (uint8_t *)argv[3],
			.psk_len = strlen(argv[3]),
		};
		wifi_connect(iface, &conf);
	}

	return CLI_CMD_SUCCESS;
}
