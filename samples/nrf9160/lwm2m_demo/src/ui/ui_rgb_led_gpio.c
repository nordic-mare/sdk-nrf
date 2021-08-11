/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_rgb_led_gpio, CONFIG_UI_LOG_LEVEL);

#define RGB_GPIO_NODE(led_label) DT_NODELABEL(led_label)
#define RGB_GPIO_PIN(led_label) DT_GPIO_PIN(RGB_GPIO_NODE(led_label), gpios)
#define RGB_GPIO_FLAGS DT_GPIO_FLAGS(RGB_GPIO_NODE(red_led), gpios)
#define RGB_GPIO_DEV_LABEL DT_GPIO_LABEL(RGB_GPIO_NODE(red_led), gpios)

static const struct device *rgb_gpio_dev;

static uint8_t red_val;
static uint8_t green_val;
static uint8_t blue_val;
static bool state;

int ui_rgb_led_gpio_on_off(bool new_state)
{
	int ret;

	state = new_state;

	ret = gpio_pin_set(rgb_gpio_dev, RGB_GPIO_PIN(red_led), state * red_val);
	if (ret) {
		LOG_ERR("Error %d: set red pin failed", ret);
		return ret;
	}
	ret = gpio_pin_set(rgb_gpio_dev, RGB_GPIO_PIN(green_led), state * green_val);
	if (ret) {
		LOG_ERR("Error %d: set green pin failed", ret);
		return ret;
	}
	ret = gpio_pin_set(rgb_gpio_dev, RGB_GPIO_PIN(blue_led), state * blue_val);
	if (ret) {
		LOG_ERR("Error %d: set blue pin failed", ret);
		return ret;
	}

	return 0;
}

int ui_rgb_led_gpio_set_colour(uint32_t colour_values)
{
	int ret;

	red_val = (uint8_t)(colour_values >> 16);
	green_val = (uint8_t)(colour_values >> 8);
	blue_val = (uint8_t)colour_values;

	if (state) {
		ret = ui_rgb_led_gpio_on_off(state);
		if (ret) {
			LOG_ERR("Error %d: LED pwm on/off failed", ret);
			return ret;
		}
	}

	return 0;
}

int ui_rgb_led_gpio_init(void)
{
	int ret;

	rgb_gpio_dev = device_get_binding(RGB_GPIO_DEV_LABEL);
	if (!rgb_gpio_dev) {
		LOG_ERR("Error %d: could not bind to LED GPIO device", -ENODEV);
		return -ENODEV;
	}

	ret = gpio_pin_configure(rgb_gpio_dev, RGB_GPIO_PIN(red_led),
				 RGB_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure red pin failed", ret);
		return ret;
	}
	ret = gpio_pin_configure(rgb_gpio_dev, RGB_GPIO_PIN(green_led),
				 RGB_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure green pin failed", ret);
		return ret;
	}
	ret = gpio_pin_configure(rgb_gpio_dev, RGB_GPIO_PIN(blue_led),
				 RGB_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure blue pin failed", ret);
		return ret;
	}

	return 0;
}
