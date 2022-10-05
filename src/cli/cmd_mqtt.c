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

#if !defined(MQTT_TEST_ENDPOINT)
#define MQTT_TEST_ENDPOINT	""
#endif
#if !defined(MQTT_TEST_CA)
#define MQTT_TEST_CA		""
#endif
#if !defined(MQTT_TEST_CERT)
#define MQTT_TEST_CERT		""
#endif
#if !defined(MQTT_TEST_KEY)
#define MQTT_TEST_KEY		""
#endif

static void on_mqtt_events(struct mqtt_client *ctx, struct mqtt_event *evt)
{
	(void)ctx;
	(void)evt;
}

static int test_mqtt_init(struct mqtt_client **mqtt)
{
	static uint8_t buf[1024];
	struct transport_conn_param transport_conf = {
		.endpoint = MQTT_TEST_ENDPOINT,
		.endpoint_len = sizeof(MQTT_TEST_ENDPOINT),
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
	transport_set_ca_cert(&transport_conf, MQTT_TEST_CA, sizeof(MQTT_TEST_CA));
	transport_set_client_cert(&transport_conf, MQTT_TEST_CERT, sizeof(MQTT_TEST_CERT));
	transport_set_client_key(&transport_conf, MQTT_TEST_KEY, sizeof(MQTT_TEST_KEY));
	// transport_set_endpoint(v, len, port)

	struct transport_interface *tls = tls_transport_create(&transport_conf);
	*mqtt = mqtt_client_create();
	mqtt_set_transport(*mqtt, tls);
	mqtt_set_rx_buffer(*mqtt, buf, sizeof(buf));
	mqtt_set_conn_param(*mqtt, &mqtt_conf);

	return mqtt_client_init(*mqtt, on_mqtt_events);
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

static void process(int argc, const char *argv[], const struct cli_io *io)
{
	static struct mqtt_client *mqtt;
	int rc = 0;

	if (argc < 2) {
		return;
	}

	if (strcmp(argv[1], "init") == 0) {
		rc = test_mqtt_init(&mqtt);
	} else if (strcmp(argv[1], "connect") == 0) {
		rc = mqtt_connect(mqtt);
	} else if (strcmp(argv[1], "disconnect") == 0) {
		rc = mqtt_disconnect(mqtt);
	} else if (strcmp(argv[1], "step") == 0) {
		rc = mqtt_step(mqtt);
	} else if (strcmp(argv[1], "listen") == 0) {
		while (1) { mqtt_step(mqtt); }
	} else if (strcmp(argv[1], "publish") == 0 && argc == 4) {
		rc = test_mqtt_publish(mqtt, argv[2], argv[3]);
	} else if (strcmp(argv[1], "subscribe") == 0 && argc == 3) {
		rc = test_mqtt_subscribe(mqtt, argv[2]);
	} else if (strcmp(argv[1], "unsubscribe") == 0 && argc == 3) {
		rc = test_mqtt_unsubscribe(mqtt, argv[2]);
	}

	char buf[16];
	snprintf(buf, sizeof(buf)-1, "RC: %d\r\n", rc);
	io->write(buf, strlen(buf));
}

cli_cmd_error_t cli_cmd_mqtt(int argc, const char *argv[], const void *env)
{
	struct cli const *cli = (struct cli const *)env;

	process(argc, argv, cli->io);

	return CLI_CMD_SUCCESS;
}
