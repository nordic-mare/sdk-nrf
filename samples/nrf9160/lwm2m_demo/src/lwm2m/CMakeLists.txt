#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_app_device.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_app_location.c)


target_sources_ifdef(CONFIG_LWM2M_APP_LIGHT_CONTROL
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_light_control.c)

target_sources_ifdef(CONFIG_LWM2M_APP_BUZZER
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_buzzer.c)

target_sources_ifdef(CONFIG_LWM2M_APP_PUSH_BUTTON
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_push_button.c)

target_sources_ifdef(CONFIG_LWM2M_APP_ONOFF_SWITCH
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_onoff_switch.c)

target_sources_ifdef(CONFIG_LWM2M_APP_LIGHT_SENSOR
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_light_sensor.c)

target_sources_ifdef(CONFIG_LWM2M_APP_ENVIRONMENT_SENSOR
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_env_sensor.c)
	
target_sources_ifdef(CONFIG_LWM2M_APP_ACCELEROMETER
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lwm2m_accelerometer.c)
