/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_led_gpio, CONFIG_UI_LOG_LEVEL);

#define LED_RED_GPIO_NODE   DT_NODELABEL(red_led)
#define LED_GREEN_GPIO_NODE DT_NODELABEL(green_led)
#define LED_BLUE_GPIO_NODE  DT_NODELABEL(blue_led)

#define LED_GPIO_NAME	    DT_GPIO_LABEL(LED_RED_GPIO_NODE, gpios)

#define LED_RED_GPIO_PIN	DT_GPIO_PIN(LED_RED_GPIO_NODE, gpios)
#define LED_GREEN_GPIO_PIN	DT_GPIO_PIN(LED_GREEN_GPIO_NODE, gpios)
#define LED_BLUE_GPIO_PIN	DT_GPIO_PIN(LED_BLUE_GPIO_NODE, gpios)

#define LED_GPIO_FLAGS	    DT_GPIO_FLAGS(LED_RED_GPIO_NODE, gpios)

static const struct device *led_gpio_dev;

static uint8_t red_val;
static uint8_t green_val;
static uint8_t blue_val;
static bool state;

int ui_led_gpio_on_off(bool new_state)
{
	int ret;

	state = new_state;
	
	ret = gpio_pin_set(led_gpio_dev, LED_RED_GPIO_PIN, state * red_val);
	if (ret) {
		LOG_ERR("Error %d: set red pin failed", ret);
		return ret;
	}
	ret = gpio_pin_set(led_gpio_dev, LED_GREEN_GPIO_PIN, state * green_val);
	if (ret) {
		LOG_ERR("Error %d: set green pin failed", ret);
		return ret;
	}
	ret = gpio_pin_set(led_gpio_dev, LED_BLUE_GPIO_PIN, state * blue_val);
	if (ret) {
		LOG_ERR("Error %d: set blue pin failed", ret);
		return ret;
	}

	return 0;
}

int ui_led_gpio_set_colour(uint32_t colour_values)
{
	int ret;

	red_val = (uint8_t)(colour_values >> 16);
	green_val = (uint8_t)(colour_values >> 8);
	blue_val = (uint8_t)colour_values;

	if (state) {
		ret = ui_led_gpio_on_off(state);
		if (ret) {
			LOG_ERR("Error %d: LED pwm on/off failed", ret);
			return ret;
		}
	}

	return 0;
}

int ui_led_gpio_init(void)
{
	int ret;

	led_gpio_dev = device_get_binding(LED_GPIO_NAME);
	if (!led_gpio_dev) {
		LOG_ERR("Error %d: could not bind to LED GPIO device", -ENODEV);
		return -ENODEV;
	}

	ret = gpio_pin_configure(led_gpio_dev, LED_RED_GPIO_PIN, 
					LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure red pin failed", ret);
		return ret;
	}
	ret = gpio_pin_configure(led_gpio_dev, LED_GREEN_GPIO_PIN, 
					LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure green pin failed", ret);
		return ret;
	}                            
	ret = gpio_pin_configure(led_gpio_dev, LED_BLUE_GPIO_PIN, 
					LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure blue pin failed", ret);
		return ret;
	}
	
	return 0;
}