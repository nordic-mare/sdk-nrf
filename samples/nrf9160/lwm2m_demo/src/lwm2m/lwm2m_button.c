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

#define BUTTON1_OBJ_INST_ID		0
#define BUTTON1_APP_NAME		"Push button 1"

/* Button 2 not supported on Thingy:91 */
#define BUTTON2_OBJ_INST_ID		1
#define BUTTON2_APP_NAME		"Push button 2"

static uint64_t btn1_counter = 0;
static uint64_t btn2_counter = 0;

int lwm2m_init_button(void)
{
	ui_button_init();

	/* create button1 object */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID));
	/* Overwriting post write callback of Digital Input State, as the original callback in the ipso object directly 
	   modifies the Digital Input Counter resource data buffer without notifying the engine, 
	   which effectively disables Value Tracking functionality for the counter resource. Won't be needed with new Zephyr update.*/ 
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID),
			NULL);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID, APPLICATION_TYPE_RID),
			BUTTON1_APP_NAME, sizeof(BUTTON1_APP_NAME), LWM2M_RES_DATA_FLAG_RO);

#ifdef CONFIG_BOARD_NRF9160DK_NRF9160NS
	/* create button1 object */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID));
	/* Overwriting post write callback of Digital Input State, as the original callback in the ipso object directly 
	   modifies the Digital Input Counter resource data buffer without notifying the engine, 
	   which effectively disables Value Tracking functionality for the counter resource. Won't be needed with new Zephyr update.*/ 
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID),
			NULL);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, APPLICATION_TYPE_RID),
			BUTTON2_APP_NAME, sizeof(BUTTON2_APP_NAME), LWM2M_RES_DATA_FLAG_RO);
#endif

	return 0;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_button_event(eh)) {
		struct button_event *event = cast_button_event(eh);

		switch (event->button_number) {
		case 1:
			lwm2m_engine_set_bool(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID), 
				event->button_state);
			
			/* Won't be needed with new Zephyr update, as the counter 
			   object is automatically updated in the ipso_push_button file. */
			if (event->button_state) {
				btn1_counter++;
				lwm2m_engine_set_u64(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID, DIGITAL_INPUT_COUNTER_RID), 
				btn1_counter);
			}
			break;

		case 2:
			lwm2m_engine_set_bool(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID), 
				event->button_state);
			
			/* Won't be needed with new Zephyr update, as the counter 
			   object is automatically updated in the ipso_push_button file. */
			if (event->button_state) {
				btn2_counter++;
				lwm2m_engine_set_u64(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, DIGITAL_INPUT_COUNTER_RID), 
				btn2_counter);
			}
			break;

		default:
			return false;
		}

		LOG_DBG("Button %d changed state to %d.", event->button_number, event->button_state);
		return true;
	}

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, button_event);