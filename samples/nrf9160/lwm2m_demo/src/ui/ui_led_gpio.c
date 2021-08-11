/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_led_gpio, CONFIG_UI_LOG_LEVEL);

#define LED_GPIO_NODE(number) DT_ALIAS(led##number)
#define LED_GPIO_PIN(number) DT_GPIO_PIN(LED_GPIO_NODE(number), gpios)
#define LED_GPIO_FLAGS DT_GPIO_FLAGS(LED_GPIO_NODE(0), gpios)
#define LED_GPIO_DEV_LABEL DT_GPIO_LABEL(LED_GPIO_NODE(0), gpios)

static const struct device *led_gpio_dev;

int ui_led_gpio_on_off(uint8_t led_num, bool new_state)
{
	int ret;
	gpio_pin_t pin;

	switch (led_num) {
	case 0:
		pin = LED_GPIO_PIN(0);
		break;

	case 1:
		pin = LED_GPIO_PIN(1);
		break;

	case 2:
		pin = LED_GPIO_PIN(2);
		break;

	case 3:
		pin = LED_GPIO_PIN(3);
		break;

	default:
		LOG_ERR("Error %d: LED %d not supported", -ENOTSUP, led_num);
		return -ENOTSUP;
	}

	ret = gpio_pin_set(led_gpio_dev, pin, new_state);
	if (ret) {
		LOG_ERR("Error %d: set LED %u pin failed", ret, led_num);
		return ret;
	}

	return 0;
}

int ui_led_gpio_init(void)
{
	int ret;

	led_gpio_dev = device_get_binding(LED_GPIO_DEV_LABEL);
	if (!led_gpio_dev) {
		LOG_ERR("Error %d: could not bind to LED GPIO device", -ENODEV);
		return -ENODEV;
	}

	ret = gpio_pin_configure(led_gpio_dev, LED_GPIO_PIN(0),
				 LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure LED 0 pin failed", ret);
		return ret;
	}
	ret = gpio_pin_configure(led_gpio_dev, LED_GPIO_PIN(1),
				 LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure LED 1 pin failed", ret);
		return ret;
	}
	ret = gpio_pin_configure(led_gpio_dev, LED_GPIO_PIN(2),
				 LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure LED 2 pin failed", ret);
		return ret;
	}
	ret = gpio_pin_configure(led_gpio_dev, LED_GPIO_PIN(3),
				 LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		LOG_ERR("Error %d: configure LED 3 pin failed", ret);
		return ret;
	}

	return 0;
}
