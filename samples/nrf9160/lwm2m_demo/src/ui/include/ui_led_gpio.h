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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Turn the LEDs on or off. If false the 
 * LEDs will be dark regardless of colour value.
 * 
 * @param new_state The LEDs' new state.
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_gpio_on_off(bool new_state);

/**
 * @brief Set the colour of the LEDs. 
 * 
 * @param[in] colour_values 32-bit number where each byte represents
 * the colour value of a specific channel. 
 * Bits 0-7 = B, bits 8-15 = G and bits 16-23 = R.
 * If a colours byte is different from 0, the corresponding pin
 * is pulled high.
 *  
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_gpio_set_colour(uint32_t colour_values);

/**
 * @brief Initialize the LEDs to use GPIO.
 * 
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_gpio_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_LED_H__ */
