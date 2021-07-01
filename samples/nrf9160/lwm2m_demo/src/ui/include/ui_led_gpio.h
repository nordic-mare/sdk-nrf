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

#ifndef UI_LED_GPIO_H__
#define UI_LED_GPIO_H__

#include <zephyr.h>
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_led_gpio_init(void);

int ui_led_gpio_on_off(bool new_state);

int ui_led_gpio_set_colour(uint32_t colour_values);

#ifdef __cplusplus
}
#endif

#endif /* UI_LED_H__ */
