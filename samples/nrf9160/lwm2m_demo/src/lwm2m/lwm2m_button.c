/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "ui_button.h"
#include "button_event.h"

#define MODULE app_lwm2m_button

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define BUTTON1_ID			1
#define BUTTON1_APP_NAME	"Push button 1"


int lwm2m_init_button(void)
{
	ui_button_init();

	/* create button1 object */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, 0));
	lwm2m_engine_set_bool(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, 0, DIGITAL_INPUT_STATE_RID), 
			ui_button_is_active(BUTTON1_ID));
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, 0, APPLICATION_TYPE_RID),
			BUTTON1_APP_NAME, sizeof(BUTTON1_APP_NAME), LWM2M_RES_DATA_FLAG_RO);

	return 0;
}


static bool event_handler(const struct event_header *eh)
{
	if (is_button_event(eh)) {
		struct button_event *event = cast_button_event(eh);

		LOG_DBG("Button %d changed state: %d.", event->button_number, event->button_state);

		switch (event->button_number) {
		case 1:
			lwm2m_engine_set_bool(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, 0, DIGITAL_INPUT_STATE_RID), 
				event->button_state);
			break;
		
		default:
			break;
		}

		return false;
	}

	return false;
}


EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, button_event);