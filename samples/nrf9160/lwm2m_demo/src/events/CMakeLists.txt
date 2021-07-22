zephyr_include_directories(include)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_gps_event.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sensor_event.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui_input_event.c)
