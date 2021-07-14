/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "ui_buzzer.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_buzzer, CONFIG_APP_LOG_LEVEL);

#define BUZZER_APP_TYPE	"BUZZER"

static int buzzer_state_cb(uint16_t obj_inst_id,
			   uint16_t res_id, uint16_t res_inst_id,
			   uint8_t *data, uint16_t data_len,
			   bool last_block, size_t total_size)
{
	int ret;
	bool state = *(bool *)data;

	LOG_DBG("Buzzer on/off! State: %d", state);

	ret = ui_buzzer_on_off(state);
	if (ret) {
		LOG_ERR("Error %d: set buzzer on/off failed", ret);
		return ret;
	}

	return 0;
}

static int buzzer_intensity_cb(uint16_t obj_inst_id,
			   uint16_t res_id, uint16_t res_inst_id,
			   uint8_t *data, uint16_t data_len,
			   bool last_block, size_t total_size)
{
	int ret;
	uint8_t intensity = *data;

	LOG_DBG("Intensity: %u", intensity);

	if (intensity > 100) {
		LOG_ERR("Error %d: intensity too high. Max 100", -EINVAL);
		return -EINVAL;
	}

	ret = ui_buzzer_set_dutycycle(intensity);
	if (ret) {
		LOG_ERR("Error %d: set dutycycle failed", ret);
		return ret;
	}

	return 0; 
}

int lwm2m_init_buzzer(void)
{
	ui_buzzer_init();
	
	lwm2m_engine_create_obj_inst(
			LWM2M_PATH(IPSO_OBJECT_BUZZER_ID, 0));
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_BUZZER_ID, 0, DIGITAL_INPUT_STATE_RID),
			buzzer_state_cb);
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_BUZZER_ID, 0, LEVEL_RID),
			buzzer_intensity_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_BUZZER_ID, 0, APPLICATION_TYPE_RID),
			BUZZER_APP_TYPE, sizeof(BUZZER_APP_TYPE),
			LWM2M_RES_DATA_FLAG_RO);

	return 0;
}