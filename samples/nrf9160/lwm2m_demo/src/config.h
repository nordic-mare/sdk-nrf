/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#if defined(CONFIG_LWM2M_DTLS_SUPPORT)

#define SERVER_TLS_TAG 35724861
#define BOOTSTRAP_TLS_TAG 35724862

static char client_psk[] = CONFIG_APP_PSK;

#endif /* defined(CONFIG_LWM2M_DTLS_SUPPORT) */
