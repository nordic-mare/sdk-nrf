#ifndef UI_INPUT_EVENT_H__
#define UI_INPUT_EVENT_H__

#include <zephyr.h>
#include <event_manager.h>

enum ui_input_type {
	Button,
	Switch
};

struct ui_input_event {
	struct event_header header;

	enum ui_input_type type;
	uint8_t device_number;
	bool state;
};

EVENT_TYPE_DECLARE(ui_input_event);

#endif /* UI_INPUT_EVENT_H__ */