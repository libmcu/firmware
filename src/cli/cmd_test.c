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

static int test_mqtt_init(struct mqtt_client **mqtt)
{
#define AWS	""
#define CERT	""
#define KEY	""
#define CA	""
	static uint8_t buf[1024];
	struct transport_conn_param transport_conf = {
		.endpoint = AWS,
		.endpoint_len = sizeof(AWS),
		.port = 8883,
		.timeout_ms = 10000,
	};
	struct mqtt_conn_param mqtt_conf = {
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
	*mqtt = mqtt_client_create();
	mqtt_set_transport(*mqtt, tls);
	mqtt_set_rx_buffer(*mqtt, buf, sizeof(buf));
	mqtt_set_conn_param(*mqtt, &mqtt_conf);

	return mqtt_client_init(*mqtt, NULL);
}

static int test_mqtt_publish(struct mqtt_client *mqtt,
		const char *topic, const char *value)
{
	struct mqtt_message msg = {
		.topic = {
			.pathname = topic,
			.pathname_len = strlen(topic),
			.qos = 0,
		},
		.payload = {
			.value = value,
			.length = strlen(value),
		},
	};

	return mqtt_publish(mqtt, &msg, false);
}

static int test_mqtt_subscribe(struct mqtt_client *mqtt, const char *topic)
{
	struct mqtt_topic msg = {
		.pathname = topic,
		.pathname_len = strlen(topic),
		.qos = 0,
	};

	return mqtt_subscribe(mqtt, &msg, 1);
}

static int test_mqtt_unsubscribe(struct mqtt_client *mqtt, const char *topic)
{
	struct mqtt_topic msg = {
		.pathname = topic,
		.pathname_len = strlen(topic),
		.qos = 0,
	};

	return mqtt_unsubscribe(mqtt, &msg, 1);
}

static void test_mqtt(int argc, const char *argv[], const struct cli_io *io)
{
	static struct mqtt_client *mqtt;
	int rc = 0;

	if (argc < 3) {
		return;
	}

	if (strcmp(argv[2], "init") == 0) {
		rc = test_mqtt_init(&mqtt);
	} else if (strcmp(argv[2], "connect") == 0) {
		rc = mqtt_connect(mqtt);
	} else if (strcmp(argv[2], "disconnect") == 0) {
		rc = mqtt_disconnect(mqtt);
	} else if (strcmp(argv[2], "step") == 0) {
		rc = mqtt_step(mqtt);
	} else if (strcmp(argv[2], "publish") == 0 && argc == 5) {
		rc = test_mqtt_publish(mqtt, argv[3], argv[4]);
	} else if (strcmp(argv[2], "subscribe") == 0 && argc == 4) {
		rc = test_mqtt_subscribe(mqtt, argv[3]);
	} else if (strcmp(argv[2], "unsubscribe") == 0 && argc == 4) {
		rc = test_mqtt_unsubscribe(mqtt, argv[3]);
	}

	char buf[16];
	snprintf(buf, sizeof(buf)-1, "RC: %d\r\n", rc);
	io->write(buf, strlen(buf));
}

cli_cmd_error_t cli_cmd_test(int argc, const char *argv[], const void *env)
{
	struct cli const *cli = (struct cli const *)env;

	if (argc >= 2 && strcmp(argv[1], "mqtt") == 0) {
		test_mqtt(argc, argv, cli->io);
	}

	return CLI_CMD_SUCCESS;
}
