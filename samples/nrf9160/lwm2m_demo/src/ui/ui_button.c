/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <dk_buttons_and_leds.h>

#include "button_event.h"

#define MODULE ui_button

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_UI_LOG_LEVEL);

/**@brief Callback for button events from the DK buttons and LEDs library. */
static void dk_button_event_handler(uint32_t button_states, uint32_t has_changed)
{
	uint8_t btn_num;

	while (has_changed) {
		btn_num = 0;

		/* Get bit position for next button that changed state. */
		for (uint8_t i = 0; i < 32; i++) {
			if (has_changed & BIT(i)) {
				btn_num = i + 1;
				break;
			}
		}

		/* Button number has been stored, remove from bitmask. */
		has_changed &= ~(1UL << (btn_num - 1));

		struct button_event *event = new_button_event();

		event->button_number = btn_num;
		event->button_state = (button_states & BIT(btn_num - 1));

		EVENT_SUBMIT(event);
	}
}

int ui_button_init(void)
{
	int ret;

	ret = dk_buttons_init(dk_button_event_handler);
	if (ret) {
		LOG_ERR("Error %d: could not initialize buttons", ret);
		return ret;
	}

	return 0;
}