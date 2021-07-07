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

#ifndef UI_LED_PWM_H__
#define UI_LED_PWM_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

int ui_led_pwm_on_off(bool new_state);

int ui_led_pwm_set_colour(uint32_t colour_values);
int ui_led_pwm_set_dutycycle(uint8_t dutycycle);

int ui_led_pwm_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_LED_PWM_H__ */
