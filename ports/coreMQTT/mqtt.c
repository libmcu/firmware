#include "net/protocols/mqtt.h"

#include <errno.h>
#include <string.h>

#include "core_mqtt.h"
#include "core_mqtt_state.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "net/transport.h"

static struct core_mqtt_ctx {
	struct mqtt_client base;
	mqtt_event_callback_t callback;
	MQTTContext_t core_mqtt;
	bool active;
} static_instance;

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
	case MQTTRecvFailed: /* fall through */
	case MQTTSendFailed:
		return -EIO;
	case MQTTKeepAliveTimeout:
		return -ETIMEDOUT;
	case MQTTIllegalState:
		return -EFAULT;
	default:
		return -EAGAIN;
	}
}

static enum mqtt_event_type get_evt_from_packet_type(uint8_t packet_type)
{
	switch (packet_type) {
	case MQTT_PACKET_TYPE_CONNACK:
		return MQTT_EVT_CONNACK;
	case MQTT_PACKET_TYPE_PUBLISH:
		return MQTT_EVT_PUBLISH;
	case MQTT_PACKET_TYPE_PUBACK:
		return MQTT_EVT_PUBACK;
	case MQTT_PACKET_TYPE_PUBREC:
		return MQTT_EVT_PUBREC;
	case MQTT_PACKET_TYPE_PUBREL:
		return MQTT_EVT_PUBREL;
	case MQTT_PACKET_TYPE_PUBCOMP:
		return MQTT_EVT_PUBCOMP;
	case MQTT_PACKET_TYPE_SUBACK:
		return MQTT_EVT_SUBACK;
	case MQTT_PACKET_TYPE_UNSUBACK:
		return MQTT_EVT_UNSUBACK;
	case MQTT_PACKET_TYPE_PINGRESP:
		return MQTT_EVT_PINGRESP;
	default:
		return MQTT_EVT_UNKNOWN;
	}
}

static enum mqtt_event_status get_subscribe_result(MQTTPacketInfo_t *packet)
{
	uint8_t *payload = NULL;
	size_t size = 0;

	MQTT_GetSubAckStatusCodes(packet, &payload, &size);

	switch (payload[0]) {
	case MQTTSubAckSuccessQos0:
		return MQTT_EVT_STATUS_SUBACK_QOS0;
	case MQTTSubAckSuccessQos1:
		return MQTT_EVT_STATUS_SUBACK_QOS1;
	case MQTTSubAckSuccessQos2:
		return MQTT_EVT_STATUS_SUBACK_QOS2;
	case MQTTSubAckFailure:
		return MQTT_EVT_STATUS_SUBACK_FAIL;
	default:
		return MQTT_EVT_STATUS_UNKNOWN;
	}
}

static uint32_t get_unixtime_ms(void)
{
	return xTaskGetTickCount() * 1000 / configTICK_RATE_HZ;
}

static void on_events(MQTTContext_t *pMqttContext,
		MQTTPacketInfo_t *pPacketInfo,
		MQTTDeserializedInfo_t *pDeserializedInfo)
{
	MQTTPublishInfo_t *msg = pDeserializedInfo->pPublishInfo;

	struct mqtt_event evt = {
		.type = get_evt_from_packet_type(pPacketInfo->type),
	};

	if (msg) {
		evt.message = (struct mqtt_message) {
			.topic = {
				.pathname = msg->pTopicName,
				.pathname_len = msg->topicNameLength,
			},
			.payload = {
				.value = msg->pPayload,
				.length = msg->payloadLength,
			},
		};
	}

	if (evt.type == MQTT_EVT_SUBACK) {
		evt.status = get_subscribe_result(pPacketInfo);
	}

	if (static_instance.callback) {
		static_instance.callback(&static_instance.base, &evt);
	}
}

int mqtt_publish(struct mqtt_client *client, const struct mqtt_message *msg,
		bool retain)
{
	struct core_mqtt_ctx *ctx = (struct core_mqtt_ctx *)client;

	MQTTPublishInfo_t data = {
		.qos = msg->topic.qos,
		.pTopicName = msg->topic.pathname,
		.topicNameLength = msg->topic.pathname_len,
		.pPayload = msg->payload.value,
		.payloadLength = msg->payload.length,
		.retain = retain,
	};

	MQTTStatus_t res = MQTT_Publish(&ctx->core_mqtt,
				&data, MQTT_GetPacketId(&ctx->core_mqtt));

	return get_errno_from_status(res);
}

int mqtt_subscribe(struct mqtt_client *client,
		const struct mqtt_topic *topics, uint16_t count)
{
	struct core_mqtt_ctx *ctx = (struct core_mqtt_ctx *)client;

	for (uint16_t i = 0; i < count; i++) {
		MQTTSubscribeInfo_t topic = {
			.qos = topics[i].qos,
			.pTopicFilter = topics[i].pathname,
			.topicFilterLength = topics[i].pathname_len,
		};

		MQTTStatus_t res = MQTT_Subscribe(&ctx->core_mqtt,
				&topic, 1, MQTT_GetPacketId(&ctx->core_mqtt));

		if (res != MQTTSuccess) {
			return get_errno_from_status(res);
		}
	}

	return 0;
}

int mqtt_unsubscribe(struct mqtt_client *client,
		const struct mqtt_topic *topics, uint16_t count)
{
	struct core_mqtt_ctx *ctx = (struct core_mqtt_ctx *)client;

	for (uint16_t i = 0; i < count; i++) {
		MQTTSubscribeInfo_t topic = {
			.qos = topics[i].qos,
			.pTopicFilter = topics[i].pathname,
			.topicFilterLength = topics[i].pathname_len,
		};

		MQTTStatus_t res = MQTT_Unsubscribe(&ctx->core_mqtt,
				&topic, 1, MQTT_GetPacketId(&ctx->core_mqtt));

		if (res != MQTTSuccess) {
			return get_errno_from_status(res);
		}
	}

	return 0;
}

int mqtt_connect(struct mqtt_client *client)
{
	struct transport_interface *iface = (struct transport_interface *)
			client->transport;

	if (transport_connect(iface)) {
		return -EAGAIN;
	}

	struct mqtt_message *will = client->conn_param.will;
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
		.cleanSession = client->conn_param.clean_session,
		.pClientIdentifier = client->conn_param.client_id.value,
		.clientIdentifierLength = client->conn_param.client_id.length,
		.keepAliveSeconds = client->conn_param.keepalive_sec,
	};

	struct core_mqtt_ctx *ctx = (struct core_mqtt_ctx *)client;
	bool session_present;
	MQTTStatus_t res = MQTT_Connect(&ctx->core_mqtt, &conf,
			will? &will_msg : NULL,
			client->conn_param.timeout_ms, &session_present);

	return get_errno_from_status(res);
}

int mqtt_disconnect(struct mqtt_client *client)
{
	struct core_mqtt_ctx *ctx = (struct core_mqtt_ctx *)client;
	struct transport_interface *iface = (struct transport_interface *)
			client->transport;

	MQTTStatus_t res = MQTT_Disconnect(&ctx->core_mqtt);

	if (transport_disconnect(iface)) {
		return -EAGAIN;
	}

	return get_errno_from_status(res);
}

int mqtt_step(struct mqtt_client *client)
{
	struct core_mqtt_ctx *ctx = (struct core_mqtt_ctx *)client;
	return get_errno_from_status(MQTT_ProcessLoop(&ctx->core_mqtt));
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

	struct core_mqtt_ctx *ctx = (struct core_mqtt_ctx *)client;
	ctx->callback = cb;

	MQTTStatus_t res = MQTT_Init(&ctx->core_mqtt,
			&transport, get_unixtime_ms, on_events, &buf);

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
