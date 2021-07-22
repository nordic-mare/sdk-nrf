zephyr_include_directories(include)

target_sources_ifdef(CONFIG_SENSOR_MODUL
    app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sensor_module.c
    )