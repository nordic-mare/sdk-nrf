/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef UI_RGB_LED_PWM_H__
#define UI_RGB_LED_PWM_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Turn the RGB LEDs on or off. If false the
 * LEDs will be dark regardless of colour value.
 *
 * @param new_state The RGB LEDs' new state.
 * @return int 0 if successful, negative error code if not.
 */
int ui_rgb_led_pwm_on_off(bool new_state);

/**
 * @brief Set the colour of the RGB LEDs.
 *
 * @param[in] colour_values 32-bit number where each byte represents
 * the colour value of a specific channel.
 * Bits 0-7 = B, bits 8-15 = G and bits 16-23 = R.
 *
 * @return int 0 if successful, negative error code if not.
 */
int ui_rgb_led_pwm_set_colour(uint32_t colour_values);

/**
 * @brief Set the intensity of the RGB LEDs.
 *
 * @param[in] intensity Integer between [0, 100], describing
 * a percentage of the maximum LED intensity.
 * @return int 0 if successful, negative error code if not.
 */
int ui_rgb_led_pwm_set_intensity(uint8_t intensity);

/**
 * @brief Initialize the RGB LEDs to use PWM.
 *
 * @return int 0 if successful, negative error code if not.
 */
int ui_rgb_led_pwm_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_RGB_LED_PWM_H__ */
