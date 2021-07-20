#ifndef BUTTON_EVENT_H__
#define BUTTON_EVENT_H__

#include <zephyr.h>
#include <event_manager.h>

struct button_event {
	struct event_header header;

	uint8_t button_number;
	bool button_state;
};

EVENT_TYPE_DECLARE(button_event);

#endif