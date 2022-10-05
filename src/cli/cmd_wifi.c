/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"
#include <stdio.h>
#include <string.h>
#include "net/wifi.h"
#include "libmcu/hexdump.h"
#include "libmcu/compiler.h"

#if !defined(MAX)
#define MAX(x, y)			(((x) < (y))? (y) : (x))
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
	case WIFI_SEC_TYPE_WEP:
		return "WEP";
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

static void on_wifi_events(const wifi_iface_t iface,
			   enum wifi_event evt, const void *data)
{
	unused(iface);

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
		io->write(buf, (size_t)MAX(len, 0));
		break;
	}
}

static void print_wifi_info(const wifi_iface_t iface)
{
	if (iface == NULL) {
		return;
	}

	const char *str = stringify_mode((enum wifi_mode)iface->mode);
	size_t len = (size_t)strlen(str);
	io->write(str, len);
	io->write("\r\n", 2);
	str = stringify_state((enum wifi_state)iface->state);
	len = (size_t)strlen(str);
	io->write(str, len);
	io->write("\r\n", 2);

	wifi_get_ap_info(iface, 0);
	char buf[WIFI_MAC_ADDR_LEN*2+4/*dots*/+2/*crlf*/];
	hexdump(buf, sizeof(buf), iface->mac, sizeof(iface->mac));
	io->write(buf, (size_t)strlen(buf));
	io->write("\r\n", 2);
	snprintf(buf, sizeof(buf), "%d.%d.%d.%d\r\n",
			iface->ip.v4[0], iface->ip.v4[1],
			iface->ip.v4[2], iface->ip.v4[3]);
	io->write(buf, (size_t)strlen(buf));
	snprintf(buf, sizeof(buf), "rssi %d\r\n", iface->rssi);
	io->write(buf, (size_t)strlen(buf));
}

static wifi_iface_t handle_single_param(const char *argv[], wifi_iface_t iface)
{
	if (strcmp(argv[1], "init") == 0) {
		iface = wifi_create();
		wifi_init(iface);
	} else if (strcmp(argv[1], "enable") == 0) {
		wifi_enable(iface);
		wifi_register_event_callback(iface, on_wifi_events);
	} else if (strcmp(argv[1], "disable") == 0) {
		wifi_disable(iface);
	} else if (strcmp(argv[1], "scan") == 0) {
		if (wifi_scan(iface) == 0) {
			scan_index = 0;
		}
	} else if (strcmp(argv[1], "disconnect") == 0) {
		wifi_disconnect(iface);
	}

	return iface;
}

static void handle_multi_params(int argc, const char *argv[], wifi_iface_t iface)
{
	if (strcmp(argv[1], "connect") == 0 && argc >= 3) {
		struct wifi_conf param = {
			.ssid = (const uint8_t *)argv[2],
			.ssid_len = (uint8_t)strlen(argv[2]),
			.security = WIFI_SEC_TYPE_NONE,
		};

		if (argc == 4) {
			param.psk = (const uint8_t *)argv[3];
			param.psk_len = (uint8_t)strlen(argv[3]);
			param.security = WIFI_SEC_TYPE_PSK;
		}

		if (argc == 5 && strcmp(argv[4], "wep") == 0) {
			param.security = WIFI_SEC_TYPE_WEP;
		}

		wifi_connect(iface, &param);
	}
}

cli_cmd_error_t cli_cmd_wifi(int argc, const char *argv[], const void *env)
{
	static wifi_iface_t iface;
	struct cli const *cli = (struct cli const *)env;

	io = cli->io;

	if (argc == 1) {
		print_wifi_info(iface);
	} else if (argc == 2) {
		iface = handle_single_param(argv, iface);
	} else if (argc > 1 && iface) {
		handle_multi_params(argc, argv, iface);
	}

	return CLI_CMD_SUCCESS;
}
