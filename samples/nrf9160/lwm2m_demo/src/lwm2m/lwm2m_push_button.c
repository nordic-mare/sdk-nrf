/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "ui_input.h"
#include "ui_input_event.h"
#include "lwm2m_defines.h"

#define MODULE app_lwm2m_push_button

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define BUTTON1_OBJ_INST_ID		0
#define BUTTON1_APP_NAME		"Push button 1"

/* Button 2 not supported on Thingy:91 */
#define BUTTON2_OBJ_INST_ID		1
#define BUTTON2_APP_NAME		"Push button 2"

static uint64_t btn1_counter = 0;
static uint64_t btn2_counter = 0;

#if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1)
static int32_t timestamp_btn1;
#if (CONFIG_LWM2M_IPSO_PUSH_BUTTON_INSTANCE_COUNT == 2)
static int32_t timestamp_btn2;
#endif /* if (CONFIG_LWM2M_IPSO_PUSH_BUTTON_INSTANCE_COUNT == 2) */

static void set_timestamp(uint16_t obj_inst_id)
{
	int32_t ts;
	char path[MAX_LWM2M_PATH_LEN];

	lwm2m_engine_get_s32(
			LWM2M_PATH(IPSO_OBJECT_DEVICE_ID, 0, CURRENT_TIME_RID), &ts);

	snprintk(path, MAX_LWM2M_PATH_LEN, "%d/%u/%d", 
			IPSO_OBJECT_PUSH_BUTTON_ID, obj_inst_id, TIMESTAMP_RID);

	lwm2m_engine_set_s32(path, ts);
}
#endif /* if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1) */

int lwm2m_init_push_button(void)
{
	ui_input_init();

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
	
#if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1)
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID, TIMESTAMP_RID), 
			&timestamp_btn1, sizeof(timestamp_btn1), LWM2M_RES_DATA_FLAG_RW);
#endif /* if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1) */

#if (CONFIG_LWM2M_IPSO_PUSH_BUTTON_INSTANCE_COUNT == 2)
	/* create button2 object */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID));
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID),
			NULL);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, APPLICATION_TYPE_RID),
			BUTTON2_APP_NAME, sizeof(BUTTON2_APP_NAME), LWM2M_RES_DATA_FLAG_RO);

#if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1)
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, TIMESTAMP_RID), 
			&timestamp_btn2, sizeof(timestamp_btn2), LWM2M_RES_DATA_FLAG_RW);
#endif /* if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1) */
#endif /* if (CONFIG_LWM2M_IPSO_PUSH_BUTTON_INSTANCE_COUNT == 2) */

	return 0;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_ui_input_event(eh)) {
		struct ui_input_event *event = cast_ui_input_event(eh);

		if (event->type != Button) {
			return false;
		}

		switch (event->device_number) {
		case 1:
			lwm2m_engine_set_bool(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID), 
				event->state);
			
			if (event->state) {
#if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1)
				set_timestamp(BUTTON1_OBJ_INST_ID);
#endif

				/* Won't be needed with new Zephyr update, as the counter 
				object is automatically updated in the ipso_push_button file. */
				btn1_counter++;
				lwm2m_engine_set_u64(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON1_OBJ_INST_ID, DIGITAL_INPUT_COUNTER_RID), 
				btn1_counter);
			}
			break;

		case 2:
			lwm2m_engine_set_bool(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID), 
				event->device_number);
			
			if (event->state) {
#if defined(CONFIG_LWM2M_IPSO_PUSH_BUTTON_VERSION_1_1)
				set_timestamp(BUTTON1_OBJ_INST_ID);
#endif

				/* Won't be needed with new Zephyr update, as the counter 
			  	object is automatically updated in the ipso_push_button file. */
				btn2_counter++;
				lwm2m_engine_set_u64(
				LWM2M_PATH(IPSO_OBJECT_PUSH_BUTTON_ID, BUTTON2_OBJ_INST_ID, DIGITAL_INPUT_COUNTER_RID), 
				btn2_counter);
			}
			break;

		default:
			return false;
		}

		LOG_DBG("Button %d changed state to %d.", event->device_number, event->state);
		return true;
	}

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, ui_input_event);
