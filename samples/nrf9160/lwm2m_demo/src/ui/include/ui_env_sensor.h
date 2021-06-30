#ifndef UI_ENV_SENSOR_H__
#define UI_ENV_SENSOR_H__

#include <zephyr.h>
#include <stdint.h>

int ui_env_sensor_init(void);

int ui_read_env_sensor(char *unit, enum sensor_channel channel, float32_value_t *float_val);

#endif