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

#define LED_PWM_NODE DT_NODELABEL(pwm0)
#define LED_PWM_PIN DT_PROP(LED_PWM_NODE, ch0_pin)
#define LED_PWM_FLAGS DT_PWMS_FLAGS(LED_PWM_NODE)
#define LED_PWM_DEV_LABEL DT_LABEL(LED_PWM_NODE)

#define PERIOD_USEC (USEC_PER_SEC / 100U)

#define INTENSITY_MAX 100U

static const struct device *led_pwm_dev;

static uint8_t intensity;
static bool state;

static uint32_t calculate_pulse_width(uint8_t intensity)
{
	return PERIOD_USEC * intensity / (INTENSITY_MAX);
}

int ui_led_pwm_on_off(bool new_state)
{
	int ret;
	uint32_t pulse_width;

	state = new_state;

	pulse_width = calculate_pulse_width(intensity);

	ret = pwm_pin_set_usec(led_pwm_dev, LED_PWM_PIN, PERIOD_USEC, pulse_width * state,
			       LED_PWM_FLAGS);
	if (ret != 0) {
		LOG_ERR("Error %d: set red pin failed", ret);
		return ret;
	}

	return 0;
}

int ui_led_pwm_set_intensity(uint8_t new_intensity)
{
	int ret;

	if (new_intensity > INTENSITY_MAX) {
		LOG_ERR("Error %d: intensity too large. Max 100", -EINVAL);
		return -EINVAL;
	}

	intensity = new_intensity;

	if (state) {
		ret = ui_led_pwm_on_off(state);
		if (ret) {
			LOG_ERR("Error %d: LED pwm on/off failed", ret);
			return ret;
		}
	}

	return 0;
}

int ui_led_pwm_init(void)
{
	led_pwm_dev = device_get_binding(LED_PWM_DEV_LABEL);
	if (!led_pwm_dev) {
		LOG_DBG("Error %d: could not bind to LED PWM device", -ENODEV);
		return -ENODEV;
	}

	return 0;
}
