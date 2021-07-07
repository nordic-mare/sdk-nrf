#ifndef LIGHT_SENSOR_H__
#define LIGHT_SENSOR_H__

#include <zephyr.h>

int light_sensor_init(void);

int light_sensor_read(uint32_t *measurement);
int colour_sensor_read(uint32_t *measurement);

#endif