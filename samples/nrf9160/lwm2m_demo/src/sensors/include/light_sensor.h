#ifndef LIGHT_SENSOR_H__
#define LIGHT_SENSOR_H__

#include <zephyr.h>
#include <drivers/sensor.h>

#define LIGHT_SENSOR_NUM_CHANNELS    4

/* Macros to scale light and colour measurements to uint8_t */
#define SCALE_LIGHT_MEAS(raw_value)     MIN((raw_value * 255U / CONFIG_LIGHT_SENSOR_MEASUREMENT_MAX_VALUE), UINT8_MAX)
#define SCALE_COLOUR_MEAS(raw_value)    MIN((raw_value * 255U / CONFIG_COLOUR_SENSOR_MEASUREMENT_MAX_VALUE), UINT8_MAX)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fetch samples from the light sensor, and read them into
 * an array of sensor_value structs.
 * 
 * @param[out] value  - Array of sensor_value structs. Must contain exactly 4 elements.
 * @param[in] size  - The size of the array.
 * @return int - 0 if successful, negative error code if not. Returns -EBUSY if sample fetch failed; user
 * should wait some hundred milliseconds before trying again.
 */
int light_sensor_read(struct sensor_value value[], size_t size);

/**
 * @brief Fetch samples from the light sensor, and read them into
 * an array of sensor_value structs. Used to read colour of a surface.
 * 
 * Turns the sense LED on during the sensor read operation. 
 * 
 * @param[out] value  - Array of sensor_value structs. Must contain exactly 4 elements.
 * @param[in] size  - The size of the array.
 * @return int - 0 if successful, negative error code if not. Returns -EBUSY if sample fetch failed; user
 * should wait some hundred milliseconds before trying again.
 */
int colour_sensor_read(struct sensor_value value[], size_t size);

/**
 * @brief Initialize the light sensor with the trigger settings set
 * in menuconfig/guiconfig.
 * 
 * @return int - 0 if successful, negative error code if not.
 */
int light_sensor_init(void);

#ifdef __cplusplus
}
#endif

#endif