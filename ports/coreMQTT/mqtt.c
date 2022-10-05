#include "net/protocols/mqtt.h"

#include <errno.h>
#include <string.h>

#include "core_mqtt.h"
#include "core_mqtt_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "net/transport_interface.h"

struct NetworkContext {
	int dummy;
};

static struct core_mqtt_client {
	struct mqtt_client base;
	mqtt_event_callback_t callback;
	MQTTContext_t core_mqtt;
	bool active;
} static_instance;

static void on_events(MQTTContext_t *pMqttContext,
		MQTTPacketInfo_t *pPacketInfo,
		MQTTDeserializedInfo_t *pDeserializedInfo)
{
}

static uint32_t get_unixtime_ms(void)
{
	return xTaskGetTickCount() * 1000 / configTICK_RATE_HZ;
}

static int get_errno_from_status(MQTTStatus_t status)
{
	switch (status) {
	case MQTTSuccess:
		return 0;
	case MQTTBadParameter:
		return -EINVAL;
	case MQTTNoMemory:
		return -ENOMEM;
	case MQTTBadResponse:
		return -EBADMSG;
	case MQTTServerRefused:
		return -EACCES;
	default:
		return -EAGAIN;
	}
}

#if 0
int mqtt_disconnect(struct mqtt_client *client);
int mqtt_publish(struct mqtt_client *client, const struct mqtt_message *msg);
int mqtt_subscribe(struct mqtt_client *client,
		const struct mqtt_subscription_list *params);
int mqtt_unsubscribe(struct mqtt_client *client,
		const struct mqtt_subscription_list *params);
int mqtt_step(struct mqtt_client *client);
#endif

int mqtt_connect(struct mqtt_client *client)
{
	struct mqtt_message *will = client->conn_params.will;
	MQTTPublishInfo_t will_msg = { 0, };

	if (will) {
		will_msg = (MQTTPublishInfo_t) {
			.qos = will->topic.qos,
			.pTopicName = will->topic.pathname,
			.topicNameLength = will->topic.pathname_len,
			.pPayload = will->payload.value,
			.payloadLength = will->payload.length,
		};
	}

	MQTTConnectInfo_t conf = {
		.cleanSession = client->conn_params.clean_session,
		.pClientIdentifier = client->conn_params.client_id.value,
		.clientIdentifierLength = client->conn_params.client_id.length,
		.keepAliveSeconds = client->conn_params.keepalive_sec,
	};

	struct core_mqtt_client *p = (struct core_mqtt_client *)client;
	bool session_present;
	MQTTStatus_t res = MQTT_Connect(&p->core_mqtt, &conf,
			will? &will_msg : NULL,
			client->conn_params.timeout_ms, &session_present);

	return get_errno_from_status(res);
}

struct mqtt_client *mqtt_client_create(void)
{
	if (static_instance.active) {
		return NULL;
	}

	memset(&static_instance, 0, sizeof(static_instance));
	static_instance.active = true;

	return &static_instance.base;
}

void mqtt_client_delete(struct mqtt_client *client)
{
	static_instance.active = false;
}

int mqtt_client_init(struct mqtt_client *client, mqtt_event_callback_t cb)
{
	if (client->rx_buf == NULL || client->transport == NULL) {
		return -EINVAL;
	}

	struct transport_interface *iface = (struct transport_interface *)
			client->transport;

	TransportInterface_t transport = {
		.pNetworkContext = client->transport,
		.send = (TransportSend_t)iface->write,
		.recv = (TransportRecv_t)iface->read,
	};
	MQTTFixedBuffer_t buf =  {
		.pBuffer = client->rx_buf,
		.size = client->rx_buf_size,
	};

	struct core_mqtt_client *p = (struct core_mqtt_client *)client;
	p->callback = cb;

	MQTTStatus_t res = MQTT_Init(&p->core_mqtt,
			&transport, get_unixtime_ms, on_events, &buf);

	return get_errno_from_status(res);
}
