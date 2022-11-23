/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FPL_BLE_INTERFACE_H
#define FPL_BLE_INTERFACE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <inttypes.h>

#if !defined(BLE_ADV_MAX_PAYLOAD_SIZE)
#define BLE_ADV_MAX_PAYLOAD_SIZE		\
		(37U - 6U/*advertiser address*/ - 2U/*header+length*/)
#endif
#define BLE_TIME_FOREVER			UINT32_MAX

enum ble_adv_mode {
	BLE_ADV_IND,         /**< connectable     scannable     undirected */
	BLE_ADV_DIRECT_IND,  /**< connectable     scannable     directed */
	BLE_ADV_NONCONN_IND, /**< non-connectable non-scannable undirected */
	BLE_ADV_SCAN_IND,    /**< non-connectable scannable     undirected */
};

enum ble_gap_evt {
	BLE_GAP_EVT_READY,
	BLE_GAP_EVT_CONNECT,
	BLE_GAP_EVT_DISCONNECT,
	BLE_GAP_EVT_ADV_COMPLETE,
	BLE_GAP_EVT_MTU,
	BLE_GAP_EVT_MAX,
};

enum ble_gatt_evt {
	BLE_GATT_EVT_MAX,
};

struct ble_adv_payload {
	uint8_t payload[BLE_ADV_MAX_PAYLOAD_SIZE];
	uint8_t index;
};

struct ble;
typedef void (*ble_event_callback_t)(struct ble *iface,
				     uint8_t evt, const void *msg);

struct ble_interface {
	void (*register_gap_event_callback)(struct ble *iface,
				ble_event_callback_t cb);
	void (*register_gatt_event_callback)(struct ble *iface,
				ble_event_callback_t cb);
	//int (*set_device_address)(type, addr);

	int (*adv_init)(struct ble *iface, enum ble_adv_mode mode);
	int (*adv_set_interval)(struct ble *iface,
				uint16_t min_ms, uint16_t max_ms);
	int (*adv_set_duration)(struct ble *iface, uint32_t msec);
	int (*adv_set_payload)(struct ble *iface,
				const struct ble_adv_payload *payload);
	int (*adv_set_scan_response)(struct ble *iface,
				const struct ble_adv_payload *payload);
	int (*adv_start)(struct ble *iface);
	int (*adv_stop)(struct ble *iface);

	//int (*sm_set_io_capability)(opt);
	//int (*sm_enable)(bonding, mitm, secure);
};

#if defined(__cplusplus)
}
#endif

#endif /* FPL_BLE_INTERFACE_H */
