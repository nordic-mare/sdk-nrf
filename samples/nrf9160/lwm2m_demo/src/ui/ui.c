/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <logging/log.h>

#include "ui.h"
#include "button.h"
#include "buzzer.h"
#include "led_gpio.h"
#include "led_pwm.h"

LOG_MODULE_REGISTER(ui, CONFIG_UI_LOG_LEVEL);

int ui_init(ui_callback_t cb)
{
	int err;
#ifdef CONFIG_UI_LED_USE_GPIO
	err = ui_gpio_led_init();
#endif

#ifdef CONFIG_UI_LED_USE_PWM
	err = ui_pwm_led_init();
#endif

#ifdef CONFIG_UI_BUTTON
	err = ui_button_init(cb);
#endif /* CONFIG_UI_BUTTONS */

#ifdef CONFIG_UI_BUZZER
	err = ui_buzzer_init();
	if (err) {
		LOG_ERR("Could not enable buzzer, err code: %d", err);
		return err;
	}
#endif /* CONFIG_UI_BUZZER */

	return err;
}
