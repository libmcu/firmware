/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef FPL_TRANSPORT_H
#define FPL_TRANSPORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "net/transport_interface.h"
#include <stdint.h>

struct transport_conn_params {
	const void *ca_cert;
	size_t ca_cert_len;
	const void *client_cert;
	size_t client_cert_len;
	const void *client_key;
	size_t client_key_len;
	const char *endpoint;
	size_t endpoint_len;
	uint16_t port;
};

struct transport_interface *tls_transport_create(
		const struct transport_conn_params *params);
void tls_transport_delete(struct transport_interface *instance);

#define transport_set_ca_cert(p_conf, p_ca, l)		\
		((p_conf)->ca_cert = (p_ca), (p_conf)->ca_cert_len = (l))
#define transport_set_client_cert(p_conf, p_cert, l)	\
		((p_conf)->client_cert = (p_cert), \
			(p_conf)->client_cert_len = (l))
#define transport_set_client_key(p_conf, p_key, l)	\
		((p_conf)->client_key = (p_key), (p_conf)->client_key_len = (l))
#define transport_set_endpoint(p_conf, p_url, l, p)	\
		((p_conf)->endpoint = (p_url), (p_conf)->endpoint_len = (l), \
			(p_conf)->port = (p))

#if defined(__cplusplus)
}
#endif

#endif /* FPL_TRANSPORT_H */
