/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "cli.h"
#include <stdio.h>
#include <string.h>

#include "net/protocols/mqtt.h"
#include "net/transport.h"

static void test_mqtt(void)
{
#define AWS	""
#define CERT	""
#define KEY	""
#define CA	""
	static uint8_t buf[1024];
	struct transport_conn_params transport_conf = {
		.endpoint = AWS,
		.endpoint_len = sizeof(AWS),
		.port = 8883,
	};
	struct mqtt_conn_params mqtt_conf = {
		.client_id = {
			.value = "hello",
			.length = 5,
		},
		.keepalive_sec = 60,
		.timeout_ms = 10000,
		.clean_session = true,
	};
	transport_set_ca_cert(&transport_conf, CA, sizeof(CA));
	transport_set_client_cert(&transport_conf, CERT, sizeof(CERT));
	transport_set_client_key(&transport_conf, KEY, sizeof(KEY));
	// transport_set_endpoint(v, len, port)

	struct transport_interface *tls = tls_transport_create(&transport_conf);
	struct mqtt_client *mqtt = mqtt_client_create();
	mqtt_set_transport(mqtt, tls);
	mqtt_set_rx_buffer(mqtt, buf, sizeof(buf));
	mqtt_set_conn_params(mqtt, &mqtt_conf);

	int rc = mqtt_client_init(mqtt, NULL);
	rc = mqtt_connect(mqtt);
}

cli_cmd_error_t cli_cmd_test(int argc, const char *argv[], const void *env)
{
	struct cli const *cli = (struct cli const *)env;

	if (argc == 2 && strcmp(argv[1], "mqtt") == 0) {
		test_mqtt();
	}

	return CLI_CMD_SUCCESS;
}
