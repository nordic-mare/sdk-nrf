zephyr_include_directories(include)

target_sources_ifdef(CONFIG_UI_ENVIRONMENT_SENSOR
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/env_sensor.c
	)
target_sources_ifdef(CONFIG_UI_LIGHT_SENSOR
	app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/light_sensor.c
	)