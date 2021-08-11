/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#ifndef UI_INPUT_H__
#define UI_INPUT_H__

#include <zephyr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the dk_buttons_and_leds library
 * with a callback function that submits a ui input event
 * when a input device's state changes.
 *
 * @return int 0 if successful, negative error code if not.
 */
int ui_input_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_INPUT_H__ */
