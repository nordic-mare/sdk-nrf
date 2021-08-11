/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/gps.h>
#include <net/lwm2m.h>
#include <net/lwm2m_path.h>

#include "app_gps_event.h"
#include "lwm2m_defines.h"

#define MODULE lwm2m_app_loc

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

static float32_value_t float_to_lwm2m_float(float val)
{
	float32_value_t out;

	out.val1 = (int32_t)val;
	out.val2 = (int32_t)((out.val1 - val) * LWM2M_FLOAT32_DEC_MAX);
	return out;
}

static float32_value_t double_to_lwm2m_float(double val)
{
	float32_value_t out;

	out.val1 = (int32_t)val;
	out.val2 = (int32_t)((out.val1 - val) * LWM2M_FLOAT32_DEC_MAX);
	return out;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_app_gps_event(eh)) {
		struct app_gps_event *event = cast_app_gps_event(eh);

		float32_value_t latitude = double_to_lwm2m_float(event->pvt.latitude);
		float32_value_t longitude = double_to_lwm2m_float(event->pvt.longitude);
		float32_value_t altitude = float_to_lwm2m_float(event->pvt.altitude);
		float32_value_t speed = float_to_lwm2m_float(event->pvt.speed);
		float32_value_t radius = float_to_lwm2m_float(event->pvt.accuracy);

		lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, LATITUDE_RID),
					 &latitude);
		lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, LONGITUDE_RID),
					 &longitude);
		lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, ALTITUDE_RID),
					 &altitude);
		lwm2m_engine_set_float32(
			LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, LOCATION_RADIUS_RID), &radius);
		lwm2m_engine_set_float32(
			LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, LOCATION_SPEED_RID), &speed);

		return true;
	}

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, app_gps_event);
