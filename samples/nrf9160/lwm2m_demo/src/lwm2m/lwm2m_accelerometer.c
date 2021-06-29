/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <net/lwm2m.h>
#include <math.h>

#include "ui.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_accel, CONFIG_APP_LOG_LEVEL);

#define SENSOR_UNIT_NAME		"m/s2"
#define FLIP_ACCELERATION_THRESHOLD	5.0
#define CALIBRATION_ITERATIONS		CONFIG_ACCEL_CALIBRATION_ITERATIONS
#define MEASUREMENT_ITERATIONS		CONFIG_ACCEL_ITERATIONS
#define ACCEL_INVERTED			CONFIG_ACCEL_INVERTED

#if defined(CONFIG_FLIP_POLL)
#define FLIP_POLL_INTERVAL		K_MSEC(CONFIG_FLIP_POLL_INTERVAL)
#else
#define FLIP_POLL_INTERVAL		K_NO_WAIT
#endif

#ifdef CONFIG_ACCEL_USE_SIM
#define FLIP_INPUT			CONFIG_FLIP_INPUT
#define CALIBRATION_INPUT		-1
#else
#define FLIP_INPUT			-1
#ifdef CONFIG_ACCEL_CALIBRATE
#define CALIBRATION_INPUT		CONFIG_CALIBRATION_INPUT
#else
#define CALIBRATION_INPUT		-1
#endif /* CONFIG_ACCEL_CALIBRATE */
#endif /* CONFIG_ACCEL_USE_SIM */

/**@brief Orientation states. */
enum orientation_state {
	ORIENTATION_NOT_KNOWN,   /**< Initial state. */
	ORIENTATION_NORMAL,      /**< Has normal orientation. */
	ORIENTATION_UPSIDE_DOWN, /**< System is upside down. */
	ORIENTATION_ON_SIDE      /**< System is placed on its side. */
};


/**@brief Struct containing current orientation and 3 axis acceleration data. */
struct accelerometer_sensor_data {
	struct sensor_value x; /**< x-axis acceleration [m/s^2]. */
	struct sensor_value y; /**< y-axis acceleration [m/s^2]. */
	struct sensor_value z; /**< z-axis acceleration [m/s^2]. */
	enum orientation_state orientation; /**< Current orientation. */
};

static const struct device *accel_dev;
static double accel_offset[3];
static uint32_t timestamp;
static struct sensor_value x_val, y_val, z_val;

static struct accelerometer_sensor_data read_accelerometer() {
	struct accelerometer_sensor_data sensor_data;
	int err;
	err = sensor_sample_fetch(accel_dev);
	if (err) {
		LOG_ERR("Sensor sample fetch failed");
	}
	err = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_X, &(sensor_data.x));
	if (err) {
		LOG_ERR("Accelerometer failed getting x value");
	}
	err = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_Y, &(sensor_data.y));
	if (err) {
		LOG_ERR("Accelerometer failed getting y value");
	}
	err = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_Z, &(sensor_data.z));
	if (err) {
		LOG_ERR("Accelerometer failed getting z value");
	}
	double x_temp, y_temp, z_temp;
	double x_int, y_int, z_int;
	x_temp = sensor_value_to_double(&sensor_data.x) - accel_offset[0];
	y_temp = sensor_value_to_double(&sensor_data.y) - accel_offset[1];
	z_temp = sensor_value_to_double(&sensor_data.z) - accel_offset[2];
	sensor_data.x.val2 = (int32_t) (modf(x_temp, &x_int) * 1000000);
	sensor_data.x.val1 = (int32_t) x_int;
	sensor_data.y.val2 = (int32_t) (modf(y_temp, &y_int) * 1000000);
	sensor_data.y.val1 = (int32_t) y_int;
	sensor_data.z.val2 = (int32_t) (modf(z_temp, &z_int) * 1000000);
	sensor_data.z.val1 = (int32_t) z_int;
	
	return sensor_data;
}

static void *accel_x_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	x_val = read_accelerometer().x;
	return &x_val;
}

static void *accel_y_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	y_val = read_accelerometer().y;
	return &y_val;
}

static void *accel_z_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	z_val = read_accelerometer().z;
	return &z_val;
}

static int accel_calibrate(void) {
	int err;
	struct sensor_value accel_data[3];
	double aggr_data[3] = {0};
	
	k_sleep(K_SECONDS(2));

	for (uint8_t i = 0; i < CALIBRATION_ITERATIONS; i++) 
	{
		LOG_INF("CALIBRATING %i of %i", i, CALIBRATION_ITERATIONS);
		err = sensor_sample_fetch(accel_dev);
		if (err) {
			LOG_ERR("Sensor sample fetch failed while calibrating accelerometer");
			return err;
		}
		err = sensor_channel_get(accel_dev,
				SENSOR_CHAN_ACCEL_X, &(accel_data[0]));
		err += sensor_channel_get(accel_dev,
				SENSOR_CHAN_ACCEL_Y, &(accel_data[1]));
		err += sensor_channel_get(accel_dev,
				SENSOR_CHAN_ACCEL_Z, &(accel_data[2]));

		if (err) {
			LOG_ERR("Sensor channel get failed while calibrating accelerometer");
			return err;
		}

		aggr_data[0] += sensor_value_to_double(&(accel_data[0]));
		aggr_data[1] += sensor_value_to_double(&(accel_data[1]));
		aggr_data[2] += (sensor_value_to_double(&(accel_data[2]))
			+ ((double)SENSOR_G) / 1000000.0);
	}
	accel_offset[0] = aggr_data[0] / (double)CALIBRATION_ITERATIONS;
	accel_offset[1] = aggr_data[1] / (double)CALIBRATION_ITERATIONS;
	accel_offset[2] = aggr_data[2] / (double)CALIBRATION_ITERATIONS;
	return 0;
}

int lwm2m_init_accel(void)
{
	int ret;

	accel_dev = device_get_binding(CONFIG_ACCEL_DEV_NAME);
	if (accel_dev == NULL) {
		LOG_ERR("Could not get %s device", CONFIG_ACCEL_DEV_NAME);
		return -ENOENT;
	}

	if (IS_ENABLED(CONFIG_ACCEL_CALIBRATE)) {
		ret = accel_calibrate();
		if (ret) {
			LOG_ERR("Could not calibrate accelerometer device: %d",
				ret);
			return ret;
		}
	}

	/* create accel object */
	lwm2m_engine_create_obj_inst("3313/0");
	lwm2m_engine_set_res_data("3313/0/5701",
				  SENSOR_UNIT_NAME, sizeof(SENSOR_UNIT_NAME),
				  LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_register_read_callback("3313/0/5702", accel_x_read_cb);
	lwm2m_engine_register_read_callback("3313/0/5703", accel_y_read_cb);
	lwm2m_engine_register_read_callback("3313/0/5704", accel_z_read_cb);
	lwm2m_engine_set_res_data("3313/0/5518",
				  &timestamp, sizeof(timestamp), 0);

	return 0;
}
