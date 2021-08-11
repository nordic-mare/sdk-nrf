#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

target_sources_ifdef(CONFIG_UI_INPUT
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_input.c)

target_sources_ifdef(CONFIG_UI_SENSE_LED
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_sense_led.c)

target_sources_ifdef(CONFIG_UI_BUZZER
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_buzzer.c)


if(CONFIG_BOARD_THINGY91_NRF9160NS)
target_sources_ifdef(CONFIG_UI_LED_USE_PWM
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_rgb_led_pwm.c)

target_sources_ifdef(CONFIG_UI_LED_USE_GPIO
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_rgb_led_gpio.c)
endif() # if(CONFIG_BOARD_THINGY91_NRF9160NS)


if(CONFIG_BOARD_NRF9160DK_NRF9160NS)
target_sources_ifdef(CONFIG_UI_LED_USE_PWM
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_led_pwm.c)

target_sources_ifdef(CONFIG_UI_LED_USE_GPIO
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_led_gpio.c)
endif() # if(CONFIG_BOARD_NRF9160DK_NRF9160NS)
