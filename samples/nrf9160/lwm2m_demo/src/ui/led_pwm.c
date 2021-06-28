/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/pwm.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_led_pwm, CONFIG_UI_LOG_LEVEL);

#define PWM_NODE DT_ALIAS(rgb_pwm)
#define PWM_PIN(channel) DT_PROP(PWM_NODE, ch##channel##_pin)
#define PWM_FLAGS   DT_PWMS_FLAGS(PWM_NODE)

const struct device *led_pwm_dev;

#define PERIOD_USEC	(USEC_PER_SEC / 100U)

static uint8_t current_dutycycle;
static uint8_t red_val;
static uint8_t green_val;
static uint8_t blue_val;
static bool is_on;

static int pwm_set(const struct device *led_pwm_dev, uint32_t pwm_pin,
		     uint32_t pulse_width, pwm_flags_t flags)
{
	return pwm_pin_set_usec(led_pwm_dev, pwm_pin, PERIOD_USEC,
				pulse_width, flags);
}

int ui_pwm_led_on_off(bool new_state)
{
    is_on = new_state;
    int ret;
    uint32_t pulse_red, pulse_green, pulse_blue;
    pulse_red = is_on * red_val * PERIOD_USEC * current_dutycycle / (255 * 100);
    pulse_green = is_on * green_val * PERIOD_USEC * current_dutycycle / (255 * 100);
    pulse_blue = is_on * blue_val * PERIOD_USEC * current_dutycycle / (255 * 100);
    ret = pwm_set(led_pwm_dev, PWM_PIN(0), pulse_red, PWM_FLAGS);
    if (ret != 0) {
        LOG_ERR("Error %d: red write failed\n", ret);
        return ret;
    }
    ret = pwm_set(led_pwm_dev, PWM_PIN(1), pulse_green, PWM_FLAGS);
    if (ret != 0) {
        LOG_ERR("Error %d: green write failed\n", ret);
        return ret;
    }
    ret = pwm_set(led_pwm_dev, PWM_PIN(2), pulse_blue, PWM_FLAGS);
    if (ret != 0) {
        LOG_ERR("Error %d: blue write failed\n", ret);
        return ret;
    }
    return 0;
}

int ui_pwm_led_set_colour(uint32_t colour_values)
{
    red_val = colour_values >> 16;
    green_val = colour_values >> 8;
    blue_val = colour_values;
    if (!is_on) {
        return 0;
    }
    return ui_pwm_led_on_off(is_on);
}


int ui_pwm_led_init(void)
{
    const char *dev_name = DT_LABEL(PWM_NODE);
	led_pwm_dev = device_get_binding(dev_name);

    if (!led_pwm_dev) {
		LOG_ERR("Could not bind to device %s", dev_name);
		return -ENODEV;
	}

    current_dutycycle = 100;

    return 0;
}


int ui_pwm_led_set_dutycycle(uint8_t dutycycle)
{
    current_dutycycle = dutycycle;

    return ui_pwm_led_on_off(is_on);
}
