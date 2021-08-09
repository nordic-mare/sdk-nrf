#ifndef ACCEL_EVENT_H__
#define ACCEL_EVENT_H__

#include <zephyr.h>
#include <event_manager.h>

#include "accelerometer.h"

struct accel_event {
	struct event_header header;

	struct accelerometer_sensor_data data;
	enum accel_orientation_state orientation;
};

EVENT_TYPE_DECLARE(accel_event);

#endif /* ACCEL_EVENT_H__ */