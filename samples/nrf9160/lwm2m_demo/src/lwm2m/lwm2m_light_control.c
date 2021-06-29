/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>

#ifdef CONFIG_UI_LED_USE_PWM
#include <led_pwm.h>
#endif
#ifdef CONFIG_UI_LED_USE_GPIO
#include <led_gpio.h>
#endif
#include <net/lwm2m.h>
#include <stdlib.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_light, CONFIG_APP_LOG_LEVEL);

#define LIGHT_NAME	"LED1"

static bool led_state;


/* TODO: Move to a pre write hook that can handle ret codes once available */
static int lc_on_off_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			uint8_t *data, uint16_t data_len,
			bool last_block, size_t total_size)
{

	bool led_val;
	led_val = *(bool *) data;
	if (led_val != led_state) {
#ifdef CONFIG_UI_LED_USE_PWM
		ui_pwm_led_on_off(led_val);
#endif
#ifdef CONFIG_UI_LED_USE_GPIO
		ui_gpio_led_on_off(led_val);
#endif
		led_state = led_val;
		/* TODO: Move to be set by an internal post write function */
		lwm2m_engine_set_s32("3311/0/5852", 0);
	}

	return 0;
}


static int lc_colour_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			uint8_t *data, uint16_t data_len,
			bool last_block, size_t total_size)
{
	int ret = 0;
	char *ptr;
	uint32_t colour_val = strtoul(data, &ptr, 0);
	LOG_DBG("Colour value: %x", colour_val);
#ifdef CONFIG_UI_LED_USE_PWM
	ret = ui_pwm_led_set_colour(colour_val);
#endif
#ifdef CONFIG_UI_LED_USE_GPIO
	ret = ui_gpio_led_set_colour(colour_val);
#endif
	return ret;
}


static int lc_dimmer_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			uint8_t *data, uint16_t data_len,
			bool last_block, size_t total_size)
{
	uint8_t dutycycle = *data;
	LOG_DBG("Dutycycle: %x", dutycycle);
	
#ifdef CONFIG_UI_LED_USE_PWM
	return ui_pwm_led_set_dutycycle(dutycycle);
#endif

	return 0;
}


int lwm2m_init_light_control(void)
{
	/* create light control device */
	lwm2m_engine_create_obj_inst("3311/0");
	lwm2m_engine_register_post_write_callback("3311/0/5850", lc_on_off_cb);
	lwm2m_engine_register_post_write_callback("3311/0/5706", lc_colour_cb);
	lwm2m_engine_register_post_write_callback("3311/0/5851", lc_dimmer_cb);
	lwm2m_engine_set_res_data("3311/0/5750",
				  LIGHT_NAME, sizeof(LIGHT_NAME),
				  LWM2M_RES_DATA_FLAG_RO);

	return 0;
}
