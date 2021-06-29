/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
/**@file
 *
 * @brief   LED control for the User Interface module. The module uses PWM to
 *	    control RGB colors and light intensity.
 */

#ifndef UI_LEDS_GPIO_H__
#define UI_LEDS_GPIO_H__

#include <zephyr.h>
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Initializes LEDs. */
int ui_gpio_led_init(void);

/**@brief Toggles LED state. */
int ui_gpio_led_on_off(bool new_state);

/**@brief Changes led colour. */
int ui_gpio_led_set_colour(uint32_t colour_values);

#ifdef __cplusplus
}
#endif

#endif /* UI_LEDS_H__ */
