/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/pwm.h>

#include <stdlib.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_rgb_led_pwm, CONFIG_UI_LOG_LEVEL);

#define RGB_PWM_NODE DT_ALIAS(rgb_pwm)
#define RGB_PWM_CHANNEL(channel) DT_PROP(RGB_PWM_NODE, ch##channel##_pin)
#define RGB_PWM_FLAGS DT_PWMS_FLAGS(RGB_PWM_NODE)
#define RGB_PWM_DEV_LABEL DT_LABEL(RGB_PWM_NODE)

#define PERIOD_USEC (USEC_PER_SEC / 100U)

#define COLOUR_MAX 255U
#define INTENSITY_MAX 100U

static const struct device *rgb_pwm_dev;

static uint8_t red_val;
static uint8_t green_val;
static uint8_t blue_val;
static uint8_t intensity;
static bool state;

static uint32_t calculate_pulse_width(uint8_t colour_val, uint8_t intensity)
{
	return PERIOD_USEC * colour_val * intensity / (COLOUR_MAX * INTENSITY_MAX);
}

int ui_rgb_led_pwm_on_off(bool new_state)
{
	int ret;
	uint32_t pulse_width_red, pulse_width_green, pulse_width_blue;

	state = new_state;

	pulse_width_red = calculate_pulse_width(red_val, intensity);
	pulse_width_green = calculate_pulse_width(green_val, intensity);
	pulse_width_blue = calculate_pulse_width(blue_val, intensity);

	ret = pwm_pin_set_usec(rgb_pwm_dev, RGB_PWM_CHANNEL(0), PERIOD_USEC,
			       pulse_width_red * state, RGB_PWM_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: set red pin failed", ret);
		return ret;
	}
	ret = pwm_pin_set_usec(rgb_pwm_dev, RGB_PWM_CHANNEL(1), PERIOD_USEC,
			       pulse_width_green * state, RGB_PWM_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: set green pin failed", ret);
		return ret;
	}
	ret = pwm_pin_set_usec(rgb_pwm_dev, RGB_PWM_CHANNEL(2), PERIOD_USEC,
			       pulse_width_blue * state, RGB_PWM_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: set blue pin failed", ret);
		return ret;
	}

	return 0;
}

int ui_rgb_led_pwm_set_colour(uint32_t colour_values)
{
	int ret;

	red_val = (uint8_t)(colour_values >> 16);
	green_val = (uint8_t)(colour_values >> 8);
	blue_val = (uint8_t)colour_values;

	if (state) {
		ret = ui_rgb_led_pwm_on_off(state);
		if (ret) {
			LOG_ERR("Error %d: LED pwm on/off failed", ret);
			return ret;
		}
	}

	return 0;
}

int ui_rgb_led_pwm_set_intensity(uint8_t new_intensity)
{
	int ret;

	if (new_intensity > INTENSITY_MAX) {
		LOG_ERR("Error %d: intensity too large. Max 100", -EINVAL);
		return -EINVAL;
	}

	intensity = new_intensity;

	if (state) {
		ret = ui_rgb_led_pwm_on_off(state);
		if (ret) {
			LOG_ERR("Error %d: LED pwm on/off failed", ret);
			return ret;
		}
	}

	return 0;
}

int ui_rgb_led_pwm_init(void)
{
	rgb_pwm_dev = device_get_binding(RGB_PWM_DEV_LABEL);
	if (!rgb_pwm_dev) {
		LOG_ERR("Error %d: could not bind to RGB LED PWM device", -ENODEV);
		return -ENODEV;
	}

	return 0;
}
