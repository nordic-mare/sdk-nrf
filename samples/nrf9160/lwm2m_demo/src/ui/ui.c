/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <logging/log.h>

#include "ui.h"
#include "buzzer.h"

LOG_MODULE_REGISTER(ui, CONFIG_UI_LOG_LEVEL);

int ui_init(ui_callback_t cb)
{
	int err;






#ifdef CONFIG_UI_BUZZER
	err = ui_buzzer_init();
	if (err) {
		LOG_ERR("Could not enable buzzer, err code: %d", err);
		return err;
	}
#endif /* CONFIG_UI_BUZZER */

	return err;
}
