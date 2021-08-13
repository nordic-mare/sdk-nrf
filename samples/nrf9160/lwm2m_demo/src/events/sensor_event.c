/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "sensor_event.h"

#include <stdio.h>

static const char *sensor_type_to_string(enum sensor_type type)
{
	switch (type) {
	case TemperatureSensor:
		return "Temperature sensor";

	case PressureSensor:
		return "Pressure sensor";

	case HumiditySensor:
		return "Humidity sensor";

	case GasResistanceSensor:
		return "Gas resistance sensor";

	case LightSensor:
		return "Light sensor";

	case ColourSensor:
		return "Colour sensor";

	default:
		return "";
	}
}

static int log_sensor_event(const struct event_header *eh, char *buf, size_t buf_len)
{
	struct sensor_event *event = cast_sensor_event(eh);

	return snprintf(buf, buf_len,
			"%s sensor event: sensor_value: val1 = %d, val2 = %d; unsigned_value = %d",
			sensor_type_to_string(event->type), event->sensor_value.val1,
			event->sensor_value.val2, event->unsigned_value);
}

EVENT_TYPE_DEFINE(sensor_event, false, log_sensor_event, NULL);
