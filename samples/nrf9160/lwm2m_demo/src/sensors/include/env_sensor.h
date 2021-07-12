#ifndef ENV_SENSOR_H__
#define ENV_SENSOR_H__

#include <zephyr.h>
#include <drivers/sensor.h>

int env_sensor_read_temp(struct sensor_value *temp_val);
int env_sensor_read_pressure(struct sensor_value *press_val);
int env_sensor_read_humidity(struct sensor_value *humid_val);
int env_sensor_read_gas_resistance(struct sensor_value *gas_res_val);

int env_sensor_init(void);

#endif