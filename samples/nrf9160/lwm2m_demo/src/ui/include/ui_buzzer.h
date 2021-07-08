/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
/**@file
 *
 * @brief   Buzzer control for the User Interface module. The module uses PWM to
 *	    control the buzzer output frequency.
 */

#ifndef UI_BUZZER_H__
#define UI_BUZZER_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**@brief Initialize buzzer in the user interface module. */
int ui_buzzer_init(void);

int ui_buzzer_on_off(bool new_state);

int ui_buzzer_set_frequency(uint32_t freq);
int ui_buzzer_set_dutycycle(uint8_t duty);

#ifdef __cplusplus
}
#endif

#endif /* UI_BUZZER_H__ */
