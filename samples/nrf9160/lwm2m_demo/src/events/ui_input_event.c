#include "ui_input_event.h"

#include <stdio.h>

static int log_ui_input_event(const struct event_header *eh, char *buf,
							size_t buf_len)
{
	struct ui_input_event *event = cast_ui_input_event(eh);

	return snprintf(buf, buf_len, 
					STRINGIFY(event->type) " event: device number = %d, state = %d",
					event->device_number, event->type);
}

EVENT_TYPE_DEFINE(
	ui_input_event,
	true,
	log_ui_input_event,
	NULL
);