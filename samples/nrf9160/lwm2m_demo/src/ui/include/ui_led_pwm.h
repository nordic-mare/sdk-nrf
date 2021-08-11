/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef UI_LED_PWM_H__
#define UI_LED_PWM_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Turn the LED on or off.
 *
 * @param new_state The LED's new state.
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_pwm_on_off(bool new_state);

/**
 * @brief Set the intensity of the LED.
 *
 * @param[in] intensity Integer between [0, 100], describing
 * a percentage of the maximum LED intensity.
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_pwm_set_intensity(uint8_t intensity);

/**
 * @brief Initialize the LED to use PWM.
 *
 * @return int 0 if successful, negative error code if not.
 */
int ui_led_pwm_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_LED_PWM_H__ */
