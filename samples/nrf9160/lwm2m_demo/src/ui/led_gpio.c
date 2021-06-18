/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_led_gpio, CONFIG_UI_LOG_LEVEL);

#define GPIO_RED_NODE DT_NODELABEL(red_led)
#define GPIO_GREEN_NODE DT_NODELABEL(green_led)
#define GPIO_BLUE_NODE DT_NODELABEL(blue_led)

#define LED_GPIO_PORT	DT_GPIO_LABEL(GPIO_RED_NODE, gpios)

#define RED_LED_GPIO_PIN	DT_GPIO_PIN(GPIO_RED_NODE, gpios)
#define GREEN_LED_GPIO_PIN	DT_GPIO_PIN(GPIO_GREEN_NODE, gpios)
#define BLUE_LED_GPIO_PIN	DT_GPIO_PIN(GPIO_BLUE_NODE, gpios)

#define LED_GPIO_FLAGS	DT_GPIO_FLAGS(GPIO_RED_NODE, gpios)

static const struct device *led_dev;
static bool is_on;
static uint8_t red_val;
static uint8_t green_val;
static uint8_t blue_val;

int ui_gpio_led_init(void)
{
	int ret;

	led_dev = device_get_binding(LED_GPIO_PORT);
	if (!led_dev) {
		return -ENODEV;
	}

	ret = gpio_pin_configure(led_dev, RED_LED_GPIO_PIN, LED_GPIO_FLAGS |
							GPIO_OUTPUT_INACTIVE);
    ret = gpio_pin_configure(led_dev, GREEN_LED_GPIO_PIN, LED_GPIO_FLAGS |
							GPIO_OUTPUT_INACTIVE);
    ret = gpio_pin_configure(led_dev, BLUE_LED_GPIO_PIN, LED_GPIO_FLAGS |
							GPIO_OUTPUT_INACTIVE);
	if (ret) {
		return ret;
	}

	return 0;
}

int ui_gpio_led_on_off(bool new_state)
{
    int ret;
    is_on = new_state;
    ret = gpio_pin_set(led_dev, RED_LED_GPIO_PIN, is_on * red_val);
    if (ret) {
        LOG_ERR("Could not set red led");
        return ret;
    }
    ret = gpio_pin_set(led_dev, GREEN_LED_GPIO_PIN, is_on * green_val);
    if (ret) {
        LOG_ERR("Could not set green led");
        return ret;
    }
    ret = gpio_pin_set(led_dev, BLUE_LED_GPIO_PIN, is_on * blue_val);
    if (ret) {
        LOG_ERR("Could not set blue led");
        return ret;
    }
    return 0;
}

int ui_gpio_led_set_colour(uint32_t colour_values)
{
    red_val = colour_values >> 16;
    green_val = colour_values >> 8;
    blue_val = colour_values;
    if (!is_on) {
        return 0;
    }
    return ui_gpio_led_on_off(true);
}
