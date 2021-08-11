/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>

#include "accelerometer.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(accelerometer, CONFIG_APP_LOG_LEVEL);

#if defined(CONFIG_ACCEL_USE_EXTERNAL)
#define ACCEL_NODE DT_PATH(soc, peripheral_40000000, spi_b000, adxl362_0)
#define ACCEL_DEV_LABEL DT_LABEL(ACCEL_NODE)
#elif defined(CONFIG_ACCEL_USE_SIM)
#define ACCEL_DEV_LABEL "SENSOR_SIM"
#endif

#if defined(CONFIG_ACCEL_CALIBRATE_ON_STARTUP)
#define CALIBRATION_ITERATIONS CONFIG_ACCEL_CALIBRATION_ITERATIONS
#endif

static const struct device *accel_dev;

static double accel_offset[3];

int accelerometer_read(struct accelerometer_sensor_data *data)
{
	int ret;
	double x_temp, y_temp, z_temp;

	ret = sensor_sample_fetch(accel_dev);
	if (ret) {
		LOG_ERR("Error %d: fetch sample failed", ret);
		return ret;
	}

	ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_X, &(data->x));
	if (ret) {
		LOG_ERR("Error %d: get x channel failed", ret);
		return ret;
	}
	ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_Y, &(data->y));
	if (ret) {
		LOG_ERR("Error %d: get y channel failed", ret);
		return ret;
	}
	ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_Z, &(data->z));
	if (ret) {
		LOG_ERR("Error %d: get z channel failed", ret);
		return ret;
	}

	/* Adjust for sensor bias */
	x_temp = sensor_value_to_double(&(data->x)) - accel_offset[0];
	y_temp = sensor_value_to_double(&(data->y)) - accel_offset[1];
	z_temp = sensor_value_to_double(&(data->z)) - accel_offset[2];
	sensor_value_from_double(&(data->x), x_temp);
	sensor_value_from_double(&(data->y), y_temp);
	sensor_value_from_double(&(data->z), z_temp);

	LOG_INF("%s: read x = %d.%06d m/s^2", accel_dev->name, data->x.val1, data->x.val2);
	LOG_INF("%s: read y = %d.%06d m/s^2", accel_dev->name, data->y.val1, data->y.val2);
	LOG_INF("%s: read z = %d.%06d m/s^2", accel_dev->name, data->z.val1, data->z.val2);

	return 0;
}

#if defined(CONFIG_ACCEL_CALIBRATE_ON_STARTUP)
static int accelerometer_calibrate(void)
{
	int ret;
	struct accelerometer_sensor_data accel_data;
	double aggr_data[3] = { 0 };

	/* TODO: Check if thi is needed for accurate calibration */
	k_sleep(K_SECONDS(2));

	for (int i = 0; i < CALIBRATION_ITERATIONS; i++) {
		ret = accelerometer_read(&accel_data);
		if (ret) {
			LOG_ERR("Error %d: read accelerometer failed", ret);
			return ret;
		}

		aggr_data[0] += sensor_value_to_double(&(accel_data.x));
		aggr_data[1] += sensor_value_to_double(&(accel_data.y));
		aggr_data[2] +=
			sensor_value_to_double(&(accel_data.z)) + ((double)SENSOR_G) / 1000000.0;
	}

	accel_offset[0] = aggr_data[0] / (double)CALIBRATION_ITERATIONS;
	accel_offset[1] = aggr_data[1] / (double)CALIBRATION_ITERATIONS;
	accel_offset[2] = aggr_data[2] / (double)CALIBRATION_ITERATIONS;

	return 0;
}
#endif /* if defined(CONFIG_ACCEL_CALIBRATE_ON_STARTUP) */

int accelerometer_init(void)
{
	accel_dev = device_get_binding(ACCEL_DEV_LABEL);
	if (!accel_dev) {
		LOG_ERR("Error %d: could not get accelerometer device", -ENODEV);
		return -ENODEV;
	}

#if defined(CONFIG_ACCEL_CALIBRATE_ON_STARTUP)
	int ret;

	ret = accelerometer_calibrate();
	if (ret) {
		LOG_ERR("Error %d: calibrate accelerometer failed", ret);
		return ret;
	}
#endif

	return 0;
}
