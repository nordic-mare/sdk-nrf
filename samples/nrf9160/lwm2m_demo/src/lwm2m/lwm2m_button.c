/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>

#include "ui.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_button, CONFIG_APP_LOG_LEVEL);

#define BUTTON1_NAME	"BUTTON1"

static int32_t timestamp[4];

int handle_button_events(struct ui_evt *evt)
{
	int32_t ts;
	bool state;

	if (!evt) {
		return -EINVAL;
	}

	/* get current time from device */
	lwm2m_engine_get_s32("3/0/13", &ts);
	state = (evt->type == UI_EVT_BUTTON_ACTIVE);
	switch (evt->button) {
	case UI_BUTTON_1:
		lwm2m_engine_set_bool("3347/0/5500", state);
		lwm2m_engine_set_s32("3347/0/5518", ts);
		return 0;
	}

	return -ENOENT;
}

int lwm2m_init_button(void)
{
	/* create button1 object */
	lwm2m_engine_create_obj_inst("3347/0");
	lwm2m_engine_set_bool("3347/0/5500", ui_button_is_active(UI_BUTTON_1));
	lwm2m_engine_set_res_data("3347/0/5750",
				  BUTTON1_NAME, sizeof(BUTTON1_NAME),
				  LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data("3347/0/5518",
				  &timestamp[0], sizeof(timestamp[0]), 0);

	return 0;
}
