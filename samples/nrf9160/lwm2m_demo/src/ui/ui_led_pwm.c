/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/pwm.h>
#include <stdlib.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_led_pwm, CONFIG_UI_LOG_LEVEL);

#define LED_PWM_NODE            DT_ALIAS(rgb_pwm)
#define LED_PWM_PIN(channel)    DT_PROP(LED_PWM_NODE, ch##channel##_pin)
#define LED_PWM_FLAGS           DT_PWMS_FLAGS(LED_PWM_NODE)
#define LED_PWM_NAME            DT_LABEL(LED_PWM_NODE)

#define PERIOD_USEC	            (USEC_PER_SEC / 100U)

#define COLOUR_RESOLUTION       255U
#define DUTYCYCLE_RESOLUTION    100U

static const struct device *led_pwm_dev;

static uint8_t red_val;
static uint8_t green_val;
static uint8_t blue_val;
static uint8_t current_intensity;
static bool current_state;

static uint32_t calculate_pulse_width(uint8_t colour_val)
{
	return PERIOD_USEC * current_state * colour_val * current_intensity 
			/ (COLOUR_RESOLUTION * DUTYCYCLE_RESOLUTION);
}

int ui_led_pwm_on_off(bool new_state)
{
	int ret;
	uint32_t pulse_width_red, pulse_width_green, pulse_width_blue;

	current_state = new_state;

	pulse_width_red = calculate_pulse_width(red_val);
	pulse_width_green = calculate_pulse_width(green_val);
	pulse_width_blue = calculate_pulse_width(blue_val);

	ret = pwm_pin_set_usec(led_pwm_dev, LED_PWM_PIN(0), PERIOD_USEC, pulse_width_red, LED_PWM_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: set red pin failed", ret);
		return ret;
	}
	ret = pwm_pin_set_usec(led_pwm_dev, LED_PWM_PIN(1), PERIOD_USEC, pulse_width_green, LED_PWM_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: set green pin failed", ret);
		return ret;
	}
	ret = pwm_pin_set_usec(led_pwm_dev, LED_PWM_PIN(2), PERIOD_USEC, pulse_width_blue, LED_PWM_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: set blue pin failed", ret);
		return ret;
	}

	return 0;
}

int ui_led_pwm_set_colour(uint32_t colour_values)
{
	red_val = (uint8_t)(colour_values >> 16);
	green_val = (uint8_t)(colour_values >> 8);
	blue_val = (uint8_t)colour_values;

	if (!current_state) {
		return 0;
	}

	return ui_led_pwm_on_off(current_state);
}

int ui_led_pwm_set_intensity(uint8_t intensity)
{
	current_intensity = intensity;

	if (!current_state) {
		return 0;
	}

	return ui_led_pwm_on_off(current_state);
}

int ui_led_pwm_init(void)
{
	led_pwm_dev = device_get_binding(LED_PWM_NAME);
	if (!led_pwm_dev) {
		LOG_ERR("Error %d: could not bind to LED PWM device", -ENODEV);
		return -ENODEV;
	}

	return 0;
}