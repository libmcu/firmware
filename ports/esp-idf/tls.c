#include "net/transport.h"
#include "esp_tls.h"

enum tls_state {
	TLS_STATE_UNKNOWN,
	TLS_STATE_CONNECTING,
	TLS_STATE_CONNECTED,
	TLS_STATE_DISCONNECTED,
};

struct tls_transport {
	struct transport_interface base;
	struct transport_conn_params params;
	enum tls_state state;
	esp_tls_t *ctx;
};

static bool is_connected(struct tls_transport *p)
{
	return p->state == TLS_STATE_CONNECTED;
}

static void disconnect_internal(struct tls_transport *p)
{
	p->state = TLS_STATE_DISCONNECTED;
	esp_tls_conn_destroy(p->ctx);

	p->ctx = NULL;
}

static int connect_internal(struct tls_transport *p)
{
	int rc = -ENOMEM;

	p->state = TLS_STATE_CONNECTING;

	if ((p->ctx = esp_tls_init()) == NULL) {
		goto out_err;
	}

	const esp_tls_cfg_t conf = {
		.cacert_buf = p->params.ca_cert,
		.cacert_bytes = p->params.ca_cert_len,
		.clientcert_buf = p->params.client_cert,
		.clientcert_bytes = p->params.client_cert_len,
		.clientkey_buf = p->params.client_key,
		.clientkey_bytes = p->params.client_key_len,
	};

	rc = (int)esp_tls_conn_new_sync(p->params.endpoint,
			p->params.endpoint_len, p->params.port, &conf, p->ctx);

	if (rc == 1) {
		p->state = TLS_STATE_CONNECTED;
		return 0;
	}

out_err:
	disconnect_internal(p);
	return rc;
}

static int write_internal(struct transport_interface *self,
		const void *data, size_t data_len)
{
	struct tls_transport *p = (struct tls_transport *)self;
	int rc = 0;

	if (!is_connected(p)) {
		disconnect_internal(p);
		if ((rc = connect_internal(p)) < 0) {
			goto out;
		}
	}

	rc = (int)esp_tls_conn_write(p->ctx, data, data_len);

	if (rc == ESP_TLS_ERR_SSL_WANT_READ ||
			rc == ESP_TLS_ERR_SSL_WANT_WRITE) {
		rc = 0;
	}

out:
	return rc;
}

static int read_internal(struct transport_interface *self,
		void *buf, size_t bufsize)
{
	struct tls_transport *p = (struct tls_transport *)self;
	int rc = 0;

	if (!is_connected(p)) {
		disconnect_internal(p);
		if ((rc = connect_internal(p)) < 0) {
			goto out;
		}
	}

	rc = (int)esp_tls_conn_read(p->ctx, buf, bufsize);

	if (rc == ESP_TLS_ERR_SSL_WANT_READ ||
				rc == ESP_TLS_ERR_SSL_WANT_WRITE) {
		rc = 0;
	}

out:
	return rc;
}

struct transport_interface *tls_transport_create(
		const struct transport_conn_params *params)
{
	struct tls_transport *iface =
		(struct tls_transport *)calloc(1, sizeof(*iface));

	if (iface == NULL) {
		return NULL;
	}

	memcpy(&iface->params, params, sizeof(*params));
	iface->base.write = write_internal;
	iface->base.read = read_internal;

	return &iface->base;
}

void tls_transport_delete(struct transport_interface *instance)
{
	struct tls_transport *p = (struct tls_transport *)instance;
	free(p);
}
