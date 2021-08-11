/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef APP_GPS_EVENT_H__
#define APP_GPS_EVENT_H__

#include <zephyr.h>
#include <drivers/gps.h>
#include <event_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

struct app_gps_event {
	struct event_header header;

	struct gps_pvt pvt;
};

EVENT_TYPE_DECLARE(app_gps_event);

#ifdef __cplusplus
}
#endif

#endif /* APP_GPS_EVENT_H__ */
