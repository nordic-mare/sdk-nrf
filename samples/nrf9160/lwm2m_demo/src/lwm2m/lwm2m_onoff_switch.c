#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "ui_input.h"
#include "ui_input_event.h"

#define MODULE app_lwm2m_onoff_switch

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define SWICTH1_OBJ_INST_ID		0
#define SWITCH1_APP_NAME		"On/Off Switch 1"

#define SWITCH2_OBJ_INST_ID		1
#define SWITCH2_APP_NAME		"On/Off Switch 2"

int lwm2m_init_onoff_switch(void)
{
	ui_input_init();

	/* create switch1 object */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_ONOFF_SWITCH_ID, SWICTH1_OBJ_INST_ID));
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_ONOFF_SWITCH_ID, SWICTH1_OBJ_INST_ID, APPLICATION_TYPE_RID),
			SWITCH1_APP_NAME, sizeof(SWITCH1_APP_NAME), LWM2M_RES_DATA_FLAG_RO);

	/* create switch2 object */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_ONOFF_SWITCH_ID, SWITCH2_OBJ_INST_ID));
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_ONOFF_SWITCH_ID, SWITCH2_OBJ_INST_ID, APPLICATION_TYPE_RID),
			SWITCH2_APP_NAME, sizeof(SWITCH2_APP_NAME), LWM2M_RES_DATA_FLAG_RO);

	return 0;
}


static bool event_handler(const struct event_header *eh)
{
	if (is_ui_input_event(eh)) {
		struct ui_input_event *event = cast_ui_input_event(eh);

		if (event->type != Switch) {
			return false;
		}

		switch (event->device_number) {
		case 1:
			lwm2m_engine_set_bool(
				LWM2M_PATH(IPSO_OBJECT_ONOFF_SWITCH_ID, SWICTH1_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID), 
				event->state);
			break;

		case 2:
			lwm2m_engine_set_bool(
				LWM2M_PATH(IPSO_OBJECT_ONOFF_SWITCH_ID, SWITCH2_OBJ_INST_ID, DIGITAL_INPUT_STATE_RID), 
				event->state);
			break;

		default:
			return false;
		}

		LOG_DBG("Switch %d changed state to %d.", event->device_number, event->state);
		return true;
	}

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, ui_input_event);