/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**@file
 *
 * @brief User interface module.
 *
 * Module that handles user interaction through button and buzzer.
 */

#ifndef UI_H__
#define UI_H__

#include <zephyr.h>
#include <dk_buttons_and_leds.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_BUTTON_1			1
#define UI_BUTTON_2			2
#define UI_SWITCH_1			3
#define UI_SWITCH_2			4

#define UI_LED_1			1
#define UI_LED_2			2
#define UI_LED_3			3
#define UI_LED_4			4

/**@brief UI event types. */
enum ui_evt_type {
	UI_EVT_BUTTON_ACTIVE,
	UI_EVT_BUTTON_INACTIVE
};

/**@brief UI event structure. */
struct ui_evt {
	enum ui_evt_type type;

	union {
		uint32_t button;
	};
};

/**
 * @brief UI callback handler type definition.
 *
 * @param evt Pointer to event struct.
 */
typedef void (*ui_callback_t)(struct ui_evt *evt);

/**
 * @brief Initializes the user interface module.
 *
 * @param cb UI callback handler. Can be NULL to disable callbacks.
 *
 * @return 0 on success or negative error value on failure.
 */
int ui_init(ui_callback_t cb);

/**
 * @brief Get the state of a button.
 *
 * @param button Button number.
 *
 * @return 1 if button is active, 0 if it's inactive.
 */
bool ui_button_is_active(uint32_t button);

/**
 * @brief Play the buzzer with set frequency and intensity.
 *
 * @param turn_on Determines whether the buzzer should turn on or not.
 *
 * @return 0 on success or negative error value on failure.
 */
int ui_buzzer_play(bool turn_on);

/**
 * @brief Set the buzzers frequency
 * 
 * @param frequency Frequency. If set to 0, the buzzer is disabled.
 *		    The frequency is limited to the range 100 - 10 000 Hz.
 *
 * @return 0 on success or negative error value on failure.
 */
int ui_buzzer_set_frequency(uint32_t new_frequency);

/**
 * @brief Set the buzzers intensity
 * 
 * @param intensity Intensity of the buzzer output. If set to 0, the buzzer is
 *		    disabled.
 *
 * @return 0 on success or negative error value on failure.
 */
int ui_buzzer_set_intensity(uint8_t new_intensity);

#ifdef __cplusplus
}
#endif

#endif /* UI_H__ */
