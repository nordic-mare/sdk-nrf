/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>
#include <stdlib.h>

#include "lwm2m_app_utils.h"

#if defined(CONFIG_BOARD_THINGY91_NRF9160NS) && defined(CONFIG_UI_LED_USE_PWM)
#define THINGY_PWM
#include "ui_rgb_led_pwm.h"
#elif defined(CONFIG_BOARD_THINGY91_NRF9160NS)
#define THINGY_GPIO
#include "ui_rgb_led_gpio.h"
#elif defined(CONFIG_UI_LED_USE_PWM) /* Assumes CONFIG_BOARD_NRF9160DK_NRF9160NS */
#define DK_PWM
#include "ui_led_pwm.h"
#else
#define DK_GPIO /* Assumes CONFIG_BOARD_NRF9160DK_NRF9160NS */
#include "ui_led_gpio.h"
#endif

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_light_control, CONFIG_APP_LOG_LEVEL);

#if defined(THINGY_PWM) || defined(THINGY_GPIO) || defined(DK_PWM)
#define NUM_LEDS 1
#elif defined(DK_GPIO)
#define NUM_LEDS 4
#endif

#define THINGY_PWM_APP_TYPE "PWM RGB LED controller"
#define THINGY_GPIO_APP_TYPE "GPIO RGB LED controller"
#define DK_PWM_APP_TYPE "PWM LED controller"
#define DK_GPIO_APP_TYPE "GPIO LED controller"

static bool state[NUM_LEDS];

static int lc_on_off_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id, uint8_t *data,
			uint16_t data_len, bool last_block, size_t total_size)
{
	bool new_state = *(bool *)data;

	if (new_state != state[obj_inst_id]) {
		int ret;

#if defined(THINGY_PWM)
		ret = ui_rgb_led_pwm_on_off(new_state);
#elif defined(THINGY_GPIO)
		ret = ui_rgb_led_gpio_on_off(new_state);
#elif defined(DK_PWM)
		ret = ui_led_pwm_on_off(new_state);
#elif defined(DK_GPIO)
		ret = ui_led_gpio_on_off((uint8_t)obj_inst_id, new_state);
#endif

		if (ret) {
			LOG_ERR("Error %d: set LED on/off failed", ret);
			return ret;
		}

		/* Reset on-time if transition from off to on */
		if (state[obj_inst_id] == false) {
			char path[MAX_LWM2M_PATH_LEN];

			snprintk(path, MAX_LWM2M_PATH_LEN, "%d/%u/%d", IPSO_OBJECT_LIGHT_CONTROL_ID,
				 obj_inst_id, ON_TIME_RID);
			lwm2m_engine_set_s32(path, 0);
		}

		state[obj_inst_id] = new_state;
	} /* if (new_state != state[obj_inst_id]) */

	return 0;
}

#if defined(THINGY_PWM) || defined(THINGY_GPIO)
static int lc_colour_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id, uint8_t *data,
			uint16_t data_len, bool last_block, size_t total_size)
{
	int ret;
	uint32_t colour_val = strtoul(data, NULL, 0);

#if defined(THINGY_PWM)
	ret = ui_rgb_led_pwm_set_colour(colour_val);
#elif defined(THINGY_GPIO)
	ret = ui_rgb_led_gpio_set_colour(colour_val);
#endif

	if (ret) {
		LOG_ERR("Error %d: set colour value failed", ret);
		return ret;
	}

	return 0;
}
#endif /* if defined(THINGY_PWM) || defined(THINGY_GPIO) */

#if defined(THINGY_PWM) || defined(DK_PWM)
static int lc_dimmer_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id, uint8_t *data,
			uint16_t data_len, bool last_block, size_t total_size)
{
	int ret;
	uint8_t intensity = *data;

#if defined(THINGY_PWM)
	ret = ui_rgb_led_pwm_set_intensity(intensity);
#elif defined(DK_PWM)
	ret = ui_led_pwm_set_intensity(intensity);
#endif

	if (ret) {
		LOG_ERR("Error %d: set dutycycle failed", ret);
		return ret;
	}

	return 0;
}
#endif /* if defined(THINGY_PWM) || defined(DK_PWM) */

int lwm2m_init_light_control(void)
{
#if defined(THINGY_PWM)
	state[0] = false;
	char colour_str[RGBIR_STR_LENGTH];

	ui_rgb_led_pwm_init();
	snprintk(colour_str, RGBIR_STR_LENGTH, "0xFFFFFF");

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0));
	lwm2m_engine_register_post_write_callback(
		LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, ON_OFF_RID), lc_on_off_cb);
	lwm2m_engine_register_post_write_callback(
		LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, COLOUR_RID), lc_colour_cb);
	lwm2m_engine_register_post_write_callback(
		LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, DIMMER_RID), lc_dimmer_cb);
	lwm2m_engine_set_res_data(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, APPLICATION_TYPE_RID),
				  THINGY_PWM_APP_TYPE, sizeof(THINGY_PWM_APP_TYPE),
				  LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_string(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, COLOUR_RID),
				colour_str);
	lwm2m_engine_set_u8(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, DIMMER_RID), 100);
#elif defined(THINGY_GPIO)
	state[0] = false;
	char colour_str[RGBIR_STR_LENGTH];

	ui_rgb_led_gpio_init();
	snprintk(colour_str, RGBIR_STR_LENGTH, "0x010101");

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0));
	lwm2m_engine_register_post_write_callback(
		LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, ON_OFF_RID), lc_on_off_cb);
	lwm2m_engine_register_post_write_callback(
		LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, COLOUR_RID), lc_colour_cb);
	lwm2m_engine_set_res_data(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, APPLICATION_TYPE_RID),
				  THINGY_GPIO_APP_TYPE, sizeof(THINGY_GPIO_APP_TYPE),
				  LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_string(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, COLOUR_RID),
				colour_str);
#elif defined(DK_PWM)
	state[0] = false;

	ui_led_pwm_init();

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0));
	lwm2m_engine_register_post_write_callback(
		LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, ON_OFF_RID), lc_on_off_cb);
	lwm2m_engine_register_post_write_callback(
		LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, DIMMER_RID), lc_dimmer_cb);
	lwm2m_engine_set_res_data(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, APPLICATION_TYPE_RID),
				  DK_PWM_APP_TYPE, sizeof(DK_PWM_APP_TYPE), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_u8(LWM2M_PATH(IPSO_OBJECT_LIGHT_CONTROL_ID, 0, DIMMER_RID), 100);
#elif defined(DK_GPIO)
	char path[MAX_LWM2M_PATH_LEN];

	ui_led_gpio_init();

	for (int i = 0; i < NUM_LEDS; i++) {
		state[i] = false;

		snprintk(path, MAX_LWM2M_PATH_LEN, "%d/%u", IPSO_OBJECT_LIGHT_CONTROL_ID, i);
		lwm2m_engine_create_obj_inst(path);

		snprintk(path, MAX_LWM2M_PATH_LEN, "%d/%u/%d", IPSO_OBJECT_LIGHT_CONTROL_ID, i,
			 ON_OFF_RID);
		lwm2m_engine_register_post_write_callback(path, lc_on_off_cb);

		snprintk(path, MAX_LWM2M_PATH_LEN, "%d/%u/%d", IPSO_OBJECT_LIGHT_CONTROL_ID, i,
			 APPLICATION_TYPE_RID);
		lwm2m_engine_set_res_data(path, DK_GPIO_APP_TYPE, sizeof(DK_GPIO_APP_TYPE),
					  LWM2M_RES_DATA_FLAG_RO);
	}
#endif

	return 0;
}
