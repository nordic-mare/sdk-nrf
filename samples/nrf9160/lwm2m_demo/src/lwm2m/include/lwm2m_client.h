/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef LWM2M_CLIENT_H__
#define LWM2M_CLIENT_H__

#include <zephyr.h>
#include <net/lwm2m.h>

#ifdef __cplusplus
extern "C" {
#endif
int lwm2m_app_init_device(char *serial_num);
int lwm2m_app_init_location(void);
int lwm2m_app_start_gps(void);

#if defined(CONFIG_LWM2M_APP_LIGHT_CONTROL)
int lwm2m_init_light_control(void);
#endif

#if defined(CONFIG_LWM2M_IPSO_TEMP_SENSOR)
int lwm2m_init_env_sensor(void);
#endif

#if defined(CONFIG_LWM2M_APP_LIGHT_SENSOR)
int lwm2m_init_light_sensor(void);
#endif

#if defined(CONFIG_LWM2M_APP_BUZZER)
int lwm2m_init_buzzer(void);
#endif

#if defined(CONFIG_LWM2M_APP_PUSH_BUTTON)
int lwm2m_init_button(void);
#endif

#if defined(CONFIG_LWM2M_APP_ACCELEROMETER)
#if CONFIG_FLIP_INPUT > 0
int handle_accel_events(struct ui_evt *evt);
#endif
int lwm2m_init_accel(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_CLIENT_H__ */
