#include "sensor_event.h"

#include <stdio.h>

static int log_sensor_event(const struct event_header *eh, char *buf,
							size_t buf_len)
{
	struct sensor_event *event = cast_sensor_event(eh);

	return snprintf(buf, buf_len, 
			STRINGIFY(event->type) " sensor event: val1 = ", 
			event->value.val1, event->value.val2);
}

EVENT_TYPE_DEFINE(
	sensor_event,
	true,
	log_sensor_event,
	NULL
);