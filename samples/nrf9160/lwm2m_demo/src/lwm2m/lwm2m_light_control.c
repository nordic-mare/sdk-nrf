/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>
#include <stdlib.h>

#ifdef CONFIG_UI_LED_USE_PWM
#include "ui_led_pwm.h"
#else
#include "led_gpio.h"
#endif

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_light_control, CONFIG_APP_LOG_LEVEL);

#define LIGHT_CONTROL_APP_TYPE	"LED controller"

static bool led_state;

static int lc_on_off_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			uint8_t *data, uint16_t data_len, bool last_block, size_t total_size)
{
	int ret;
	bool new_led_state = *(bool *)data;

	if (new_led_state != led_state)  {
#ifdef CONFIG_UI_LED_USE_PWM
		ret = ui_led_pwm_on_off(new_led_state);
#else
		ret = ui_led_gpio_on_off(new_led_state);
#endif /* CONFIG_UI_LED_USE_PWM */
		if (ret) {
			LOG_ERR("Error %d: set LED on/off failed", ret);
			return ret;
		}

		/* Reset on-time if transition from off to on */
		if (led_state == false) {
			lwm2m_engine_set_s32(
				LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, ON_TIME_RID), 0);
		}

		led_state = new_led_state;
	} /* if (new_led_state != led_state) */

	return 0;
}

static int lc_colour_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			uint8_t *data, uint16_t data_len, bool last_block, size_t total_size)
{
	int ret;
	uint32_t colour_val = strtoul(data, NULL, 0);

#ifdef CONFIG_UI_LED_USE_PWM
	ret = ui_led_pwm_set_colour(colour_val);
#else
	ret = ui_led_gpio_set_colour(colour_val);
#endif 
	if (ret) {
		LOG_ERR("Error %d: set colour value failed", ret);
		return ret;
	}

	return 0;
}

static int lc_dimmer_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			uint8_t *data, uint16_t data_len,
			bool last_block, size_t total_size)
{
	int ret;
	uint8_t intensity = *data;
	
#ifdef CONFIG_UI_LED_USE_PWM
	ret = ui_led_pwm_set_intensity(intensity);
	if (ret) {
		LOG_ERR("Error %d: set dutycycle failed", ret);
		return ret;
	}
#endif

	return 0;
}

int lwm2m_init_light_control(void)
{
	led_state = false;

#ifdef CONFIG_UI_LED_USE_PWM
	ui_led_pwm_init();
#else
	ui_led_gpio_init();
#endif 

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0));
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, ON_OFF_RID), lc_on_off_cb);
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, COLOUR_RID), lc_colour_cb);
	lwm2m_engine_register_post_write_callback(
			LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, DIMMER_RID), lc_dimmer_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, APPLICATION_TYPE_RID),
			LIGHT_CONTROL_APP_TYPE, sizeof(LIGHT_CONTROL_APP_TYPE),
			LWM2M_RES_DATA_FLAG_RO);

	return 0;
}