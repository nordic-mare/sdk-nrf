/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef UI_LED_GPIO_H__
#define UI_LED_GPIO_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Turn the LED on or off. If false the
 * LEDs will be dark regardless of colour value.
 *
 * @param new_state The LED's new state.
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_gpio_on_off(uint8_t led_num, bool new_state);

/**
 * @brief Initialize the LED to use GPIO.
 *
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_gpio_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_LED_H__ */
