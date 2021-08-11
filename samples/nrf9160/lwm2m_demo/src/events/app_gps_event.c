/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "app_gps_event.h"

#include <stdio.h>

static int log_gps_event(const struct event_header *eh, char *buf, size_t buf_len)
{
	struct app_gps_event *event = cast_app_gps_event(eh);

	return snprintf(buf, buf_len, "gps_event lat: %d long: %d alt: %d",
			(int)event->pvt.latitude, (int)event->pvt.longitude,
			(int)event->pvt.altitude);
}

EVENT_TYPE_DEFINE(app_gps_event, true, log_gps_event, NULL);
