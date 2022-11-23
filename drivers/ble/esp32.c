/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "net/ble.h"
#include "net/ble_common_data_type.h"

#include <errno.h>
#include <string.h>

#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/util/util.h"

#include "libmcu/logging.h"
#include "libmcu/hexdump.h"
#include "libmcu/assert.h"
#include "libmcu/compiler.h"

LIBMCU_STATIC_ASSERT(BLE_GAP_EVT_MAX < UINT8_MAX, "");

struct ble {
	struct ble_interface api;

	ble_event_callback_t gap_event_callback;
	ble_event_callback_t gatt_event_callback;

	struct {
		enum ble_adv_mode mode;
		uint16_t min_ms;
		uint16_t max_ms;
		uint32_t duration_ms;

		struct ble_adv_payload payload;
		struct ble_adv_payload scan_response;
	} adv;

	uint8_t addr_type;
};

static volatile bool ready;
static uint8_t adv_addr_type;

static int on_gap_event(struct ble_gap_event *event, void *arg)
{
	struct ble *iface = (struct ble *)arg;
	struct ble_gap_conn_desc desc;

	debug("GAP event %u", event->type);

	switch (event->type) {
	case BLE_GAP_EVENT_CONNECT:
	case BLE_GAP_EVENT_DISCONNECT:
	case BLE_GAP_EVENT_CONN_UPDATE:
	case BLE_GAP_EVENT_ADV_COMPLETE:
	case BLE_GAP_EVENT_MTU:
	default:
		break;
	}

	if (iface && iface->gap_event_callback) {
		//iface->gap_event_callback()
	}

	return 0;
}

static void on_gatt_register(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
	char buf[BLE_UUID_STR_LEN];

	switch (ctxt->op) {
	case BLE_GATT_REGISTER_OP_SVC:
		debug("registered service %s with handle=%d\n",
				ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
				ctxt->svc.handle);
		break;
	case BLE_GATT_REGISTER_OP_CHR:
		debug("registered characteristic %s with "
				"def_handle=%d val_handle=%d\n",
				ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
				ctxt->chr.def_handle, ctxt->chr.val_handle);
		break;
	case BLE_GATT_REGISTER_OP_DSC:
		debug("registered descriptor %s with handle=%d\n",
				ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
				ctxt->dsc.handle);
		break;
	default:
		assert(0);
		break;
	}
}

static void on_reset(int reason)
{
	info("reset reason: %d", reason);
}

static void on_sync(void)
{
	int rc = ble_hs_util_ensure_addr(0);
	assert(rc == 0);

	rc = ble_hs_id_infer_auto(0, &adv_addr_type);
	if (rc != 0) {
		error("error determining address type; rc=%d", rc);
		return;
	}

	uint8_t addr_val[6] = {0};
	rc = ble_hs_id_copy_addr(adv_addr_type, addr_val, NULL);

	char buf[12];
	hexdump(buf, sizeof(buf), addr_val, sizeof(addr_val));
	info("Device Address: %.*s, type %d", sizeof(buf), buf, adv_addr_type);

	ready = true;
}

static void ble_spp_server_host_task(void *param)
{
	nimble_port_run();
	nimble_port_freertos_deinit();
}

static int adv_set_interval(struct ble *iface, uint16_t min_ms, uint16_t max_ms)
{
	assert(min_ms >= BLE_ADV_MIN_INTERVAL_MS &&
			max_ms <= BLE_ADV_MAX_INTERVAL_MS);

	iface->adv.min_ms = min_ms;
	iface->adv.max_ms = max_ms;

	return 0;
}

static int adv_set_duration(struct ble *iface, uint32_t msec)
{
	iface->adv.duration_ms = msec;
	return 0;
}

static void register_gap_event_callback(struct ble *iface,
					ble_event_callback_t cb)
{
	iface->gap_event_callback = cb;
}

static void register_gatt_event_callback(struct ble *iface,
					 ble_event_callback_t cb)
{
	iface->gatt_event_callback = cb;
}

static int adv_set_payload(struct ble *iface,
			   const struct ble_adv_payload *payload)
{
	memcpy(&iface->adv.payload, payload, sizeof(*payload));
	return 0;
}

static int adv_set_scan_response(struct ble *iface,
				 const struct ble_adv_payload *payload)
{
	memcpy(&iface->adv.scan_response, payload, sizeof(*payload));
	return 0;
}

static int adv_start(struct ble *iface)
{
	if (!ready) {
		return -EAGAIN;
	}

	int rc = ble_gap_adv_set_data(iface->adv.payload.payload,
			       iface->adv.payload.index);
	rc |= ble_gap_adv_rsp_set_data(iface->adv.scan_response.payload,
			       iface->adv.scan_response.index);
	if (rc != 0) {
		return -EINVAL;
	}

	struct ble_gap_adv_params adv_params = {
		.conn_mode = BLE_GAP_CONN_MODE_UND,
		.disc_mode = BLE_GAP_DISC_MODE_GEN,
		.itvl_min = iface->adv.min_ms * 1000 / BLE_HCI_ADV_ITVL,
		.itvl_max = iface->adv.max_ms * 1000 / BLE_HCI_ADV_ITVL,
		//.channel_map = , // 1=37, 2=38, 4=39, 7=all
		//.filter_policy = ,
		//.high_duty_cycle = , // directed
	};

	switch (iface->adv.mode) {
	case BLE_ADV_DIRECT_IND:
		adv_params.conn_mode = BLE_GAP_CONN_MODE_DIR;
		adv_params.disc_mode = BLE_GAP_DISC_MODE_LTD;
		break;
	case BLE_ADV_NONCONN_IND:
		adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
		adv_params.disc_mode = BLE_GAP_DISC_MODE_NON;
		break;
	case BLE_ADV_SCAN_IND:
		adv_params.conn_mode = BLE_GAP_CONN_MODE_NON;
		break;
	default:
	case BLE_ADV_IND:
		break;
	}

	rc = ble_gap_adv_start(adv_addr_type, NULL,
			(int32_t)iface->adv.duration_ms,
			&adv_params, on_gap_event, iface);
	if (rc != 0) {
		error("adv not started: %d", rc);
		return -EFAULT;
	}

	return 0;
}

static int adv_stop(struct ble *iface)
{
	return 0;
}

static int adv_init(struct ble *iface, enum ble_adv_mode mode)
{
	memset(&iface->adv, 0, sizeof(iface->adv));

	iface->adv.mode = mode;

	iface->adv.min_ms = 0;
	iface->adv.max_ms = 0;
	iface->adv.duration_ms = BLE_HS_FOREVER;

	return 0;
}

static void initialize(struct ble *iface)
{
	ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
	nimble_port_init();

	ble_hs_cfg.reset_cb = on_reset;
	ble_hs_cfg.sync_cb = on_sync;
	ble_hs_cfg.gatts_register_cb = on_gatt_register;
	ble_hs_cfg.gatts_register_arg = iface;
	ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

	ble_svc_gap_init();
	ble_svc_gatt_init();

	nimble_port_freertos_init(ble_spp_server_host_task);
}

struct ble *esp_ble_create(void)
{
	static struct ble iface = {
		.api = {
			.adv_init = adv_init,
			.register_gap_event_callback = register_gap_event_callback,
			.register_gatt_event_callback = register_gatt_event_callback,
			.adv_set_interval = adv_set_interval,
			.adv_set_duration = adv_set_duration,
			.adv_set_payload = adv_set_payload,
			.adv_set_scan_response = adv_set_scan_response,
			.adv_start = adv_start,
			.adv_stop = adv_stop,
		},
	};

	initialize(&iface);

	return &iface;
}
