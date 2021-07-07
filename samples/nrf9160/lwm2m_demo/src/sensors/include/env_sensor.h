#ifndef ENV_SENSOR_H__
#define ENV_SENSOR_H__

#include <zephyr.h>

int env_sensor_init(void);

int env_sensor_read_temp(int32_t *temp_float_val1, int32_t *temp_float_val2);
int env_sensor_read_pressure(int32_t *press_float_val1, int32_t *press_float_val2);
int env_sensor_read_humidity(int32_t *humid_float_val1, int32_t *humid_float_val2);
int env_sensor_read_gas_resistance(int32_t *gas_res_float_val1, int32_t *gas_res_float_val2);

#endif