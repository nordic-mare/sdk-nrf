#ifndef UI_LIGHT_SENSOR_H__
#define UI_LIGHT_SENSOR_H__

#include <zephyr.h>

int ui_light_sensor_init(void);

int ui_light_sensor_read(uint32_t *measurement);
int ui_colour_sensor_read(uint32_t *measurement);

#endif