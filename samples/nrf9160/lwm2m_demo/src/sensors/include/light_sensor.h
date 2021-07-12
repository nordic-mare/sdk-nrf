#ifndef LIGHT_SENSOR_H__
#define LIGHT_SENSOR_H__

#include <zephyr.h>
#include <drivers/sensor.h>

int light_sensor_read(struct sensor_value *value);
int colour_sensor_read(struct sensor_value *value);

int light_sensor_init(void);

#endif