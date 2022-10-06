/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file wifi.h
 * @note The APIs should be called in the same context, running by single
 *	 thread because it doesn't care about race conditions.
 */

#ifndef FPL_WIFI_H
#define FPL_WIFI_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#define WIFI_SSID_MAX_LEN		32
#define WIFI_MAC_ADDR_LEN		6

#if !defined(wifi_init)
#define wifi_init			fpl_wifi_init
#endif
#if !defined(wifi_deinit)
#define wifi_deinit			fpl_wifi_deinit
#endif
#if !defined(wifi_connect)
#define wifi_connect			fpl_wifi_connect
#endif
#if !defined(wifi_disconnect)
#define wifi_disconnect			fpl_wifi_disconnect
#endif

enum wifi_event {
	WIFI_EVT_UNKNOWN,
	WIFI_EVT_CONNECTED,
	WIFI_EVT_DISCONNECTED,
	WIFI_EVT_SCAN_RESULT,
	WIFI_EVT_SCAN_DONE,
	WIFI_EVT_STARTED,
};

enum wifi_mode {
	WIFI_MODE_INFRA,
	WIFI_MODE_ACCESS_POINT,
	WIFI_MODE_MESH,
};

enum wifi_state {
	WIFI_STATE_UNKNOWN,
	WIFI_STATE_DISABLED,
	WIFI_STATE_DISCONNECTING,
	WIFI_STATE_DISCONNECTED,
	WIFI_STATE_INACTIVE,
	WIFI_STATE_SCANNING,
	WIFI_STATE_AUTHENTICATING,
	WIFI_STATE_ASSOCIATING,
	WIFI_STATE_ASSOCIATED,
};

enum wifi_security {
	WIFI_SEC_TYPE_UNKNOWN,
	WIFI_SEC_TYPE_NONE,
	WIFI_SEC_TYPE_WEP,
	WIFI_SEC_TYPE_PSK,
	WIFI_SEC_TYPE_PSK_SHA256,
	WIFI_SEC_TYPE_PSK_SAE,
};

enum wifi_frequency_band {
	WIFI_FREQ_2_4_GHZ,
	WIFI_FREQ_5_GHZ,
	WIFI_FREQ_6_GHZ,
};

enum wifi_mfp {
	WIFI_MFP_DISABLED,
	WIFI_MFP_OPTIONAL,
	WIFI_MFP_REQUIRED,
};

struct wifi_iface;
typedef void (*wifi_event_callback_t)(const struct wifi_iface *iface,
				    enum wifi_event evt, const void *data);

struct wifi_iface {
	volatile uint8_t state; /**< @ref wifi_iface_state */
	volatile uint8_t state_prev;
	volatile uint8_t mode;  /**< @ref wifi_mode */
	volatile uint8_t mode_prev;

	wifi_event_callback_t callbacks;

	uint8_t mac[WIFI_MAC_ADDR_LEN];
	int8_t rssi;
	int8_t padding;

	struct {
		uint8_t v4[4];
		uint32_t v6[4];
	} ip;
};

struct wifi_conf {
	const uint8_t *ssid;
	uint8_t ssid_len;
	const uint8_t *psk;
	uint8_t psk_len;
	const uint8_t *sae;
	uint8_t sae_len;

	enum wifi_frequency_band band;
	uint8_t channel;
	enum wifi_security security;
	enum wifi_mfp mfp;

	int timeout_ms;
};

struct wifi_scan_result {
	uint8_t ssid[WIFI_SSID_MAX_LEN];
	uint8_t ssid_len;

	enum wifi_frequency_band band;
	uint8_t channel;
	enum wifi_security security;
	enum wifi_mfp mfp;
	int8_t rssi;

	uint8_t mac[WIFI_MAC_ADDR_LEN];
	uint8_t mac_len;
};

struct wifi_ap_info {
	uint8_t ssid[WIFI_SSID_MAX_LEN];
	uint8_t ssid_len;

	uint8_t mac[WIFI_MAC_ADDR_LEN];
	uint8_t mac_len;

	uint8_t channel;
	int8_t rssi;
	enum wifi_security security;
};

int wifi_connect(struct wifi_iface *iface, const struct wifi_conf *param);
int wifi_disconnect(struct wifi_iface *iface);
int wifi_scan(struct wifi_iface *iface);
int wifi_get_ap_info(struct wifi_iface *iface, struct wifi_ap_info *info);

struct wifi_iface *wifi_create(void);
void wifi_delete(struct wifi_iface *iface);
int wifi_init(struct wifi_iface *iface);
int wifi_deinit(struct wifi_iface *iface);
int wifi_enable(struct wifi_iface *iface);
int wifi_disable(struct wifi_iface *iface);

int wifi_register_event_callback(struct wifi_iface *iface,
				 const wifi_event_callback_t cb);

void wifi_raise_event(struct wifi_iface *iface, enum wifi_event evt);
void wifi_raise_event_with_data(struct wifi_iface *iface,
				enum wifi_event evt, const void *data);

void wifi_set_state(struct wifi_iface *iface, enum wifi_state state);
void wifi_set_mode(struct wifi_iface *iface, enum wifi_mode mode);

#if 0
int wifi_add_event_callback(struct wifi_iface *iface,
			    enum wifi_event evt,
			    const wifi_event_callback_t cb);
wifi_enable_ap()
wifi_disable_ap()
#endif

#if defined(__cplusplus)
}
#endif

#endif /* FPL_WIFI_H */
