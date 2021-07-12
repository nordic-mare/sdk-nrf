#include "button_event.h"

#include <stdio.h>

static int log_button_event(const struct event_header *eh, char *buf,
							size_t buf_len)
{
	struct button_event *event = cast_button_event(eh);

	return snprintf(buf, buf_len, "button_number=%d, button_state=%d", event->button_number, event->button_state);
}

EVENT_TYPE_DEFINE(
	button_event,
	true,
	log_button_event,
	NULL
);