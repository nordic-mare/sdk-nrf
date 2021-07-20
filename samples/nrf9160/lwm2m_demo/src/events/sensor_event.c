#include "sensor_event.h"

#include <stdio.h>

static int log_sensor_event(const struct event_header *eh, char *buf,
							size_t buf_len)
{
	struct sensor_event *event = cast_sensor_event(eh);

	return snprintf(buf, buf_len, 
			STRINGIFY(event->type) " sensor event: sensor_value: val1 = %d, val2 = %d; unsigned_value = %d", 
			event->sensor_value.val1, event->sensor_value.val2, event->unsigned_value);
}

EVENT_TYPE_DEFINE(
	sensor_event,
	true,
	log_sensor_event,
	NULL
);