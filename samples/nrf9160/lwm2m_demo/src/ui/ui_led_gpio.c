/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_led_gpio, CONFIG_UI_LOG_LEVEL);

#define RED_LED_GPIO_NODE DT_NODELABEL(red_led)
#define GREEN_LED_GPIO_NODE DT_NODELABEL(green_led)
#define BLUE_LED_GPIO_NODE DT_NODELABEL(blue_led)

#define GPIO_NAME	DT_GPIO_LABEL(RED_LED_GPIO_NODE, gpios)

#define RED_LED_GPIO_PIN	DT_GPIO_PIN(RED_LED_GPIO_NODE, gpios)
#define GREEN_LED_GPIO_PIN	DT_GPIO_PIN(GREEN_LED_GPIO_NODE, gpios)
#define BLUE_LED_GPIO_PIN	DT_GPIO_PIN(BLUE_LED_GPIO_NODE, gpios)

#define LED_GPIO_FLAGS	DT_GPIO_FLAGS(RED_LED_GPIO_NODE, gpios)

static const struct device *gpio_dev;
static uint8_t red_val;
static uint8_t green_val;
static uint8_t blue_val;
static bool is_on;

int ui_led_gpio_init(void)
{
	int ret;

	gpio_dev = device_get_binding(GPIO_NAME);
	if (!gpio_dev) {
        LOG_ERR("Could not bind to GPIO device");
		return -ENODEV;
	}

	ret = gpio_pin_configure(gpio_dev, RED_LED_GPIO_PIN, LED_GPIO_FLAGS |
							GPIO_OUTPUT_INACTIVE);
    if (ret) {
        LOG_ERR("Could not configure red LED GPIO pin");
		return ret;
	}
    ret = gpio_pin_configure(gpio_dev, GREEN_LED_GPIO_PIN, LED_GPIO_FLAGS |
							GPIO_OUTPUT_INACTIVE);
    if (ret) {
        LOG_ERR("Could not configure green LED GPIO pin");
		return ret;
	}                            
    ret = gpio_pin_configure(gpio_dev, BLUE_LED_GPIO_PIN, LED_GPIO_FLAGS |
							GPIO_OUTPUT_INACTIVE);
	if (ret) {
        LOG_ERR("Could not configure blue LED GPIO pin");
		return ret;
	}

	return 0;
}

int ui_led_gpio_on_off(bool new_state)
{
    int ret;
    is_on = new_state;
    
    ret = gpio_pin_set(gpio_dev, RED_LED_GPIO_PIN, is_on * red_val);
    if (ret) {
        LOG_ERR("Could not set red led");
        return ret;
    }
    ret = gpio_pin_set(gpio_dev, GREEN_LED_GPIO_PIN, is_on * green_val);
    if (ret) {
        LOG_ERR("Could not set green led");
        return ret;
    }
    ret = gpio_pin_set(gpio_dev, BLUE_LED_GPIO_PIN, is_on * blue_val);
    if (ret) {
        LOG_ERR("Could not set blue led");
        return ret;
    }
    return 0;
}

int ui_led_gpio_set_colour(uint32_t colour_values)
{
    red_val = (uint8_t)(colour_values >> 16);
    green_val = (uint8_t)(colour_values >> 8);
    blue_val = (uint8_t)colour_values;
    if (!is_on) {
        return 0;
    }
    return ui_led_gpio_on_off(true);
}
