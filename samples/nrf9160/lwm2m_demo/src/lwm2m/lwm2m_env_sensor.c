/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <net/lwm2m.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_env_sens, CONFIG_APP_LOG_LEVEL);

#define ENV_SENSOR_NODE	DT_PATH(soc, peripheral_40000000, i2c_a000, bme680_76)
#define ENV_SENSOR_NAME	DT_LABEL(ENV_SENSOR_NODE)
/* Default values if sensors not available */
static struct float32_value temp_float = { 25, 500000 };
static struct float32_value press_float = { 100, 500000 };
static struct float32_value humid_float = { 45, 500000 };

static char *temp_unit = "C";
static char *press_unit = "kPa";
static char *humid_unit = "%";

static const struct device *die_dev;
static int32_t timestamp;

#if defined(ENV_SENSOR_NAME)
static int read_enviroment_sensor(const struct device *sens_dev, char *unit,
                enum sensor_channel channel, struct float32_value *float_val)
{
	const char *name = sens_dev->name;
	struct sensor_value sensor_val;
	int ret;

	ret = sensor_sample_fetch(sens_dev);
	if (ret) {
		LOG_ERR("%s: I/O error: %d", name, ret);
		return ret;
	}

	ret = sensor_channel_get(sens_dev, channel, &sensor_val);
	if (ret) {
		LOG_ERR("%s: can't get data: %d", name, ret);
		return ret;
	}

	LOG_DBG("%s: read %d.%d %s", name, sensor_val.val1, sensor_val.val2, unit);
	float_val->val1 = sensor_val.val1;
	float_val->val2 = sensor_val.val2;

	return 0;
}
#endif

static void *temp_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	int32_t ts;

	/* Only object instance 0 is currently used */
	if (obj_inst_id != 0) {
		*data_len = 0;
		return NULL;
	}

#if defined(ENV_SENSOR_NAME)
	/*
	 * No need to check if read was successful, just reuse the
	 * previous value which is already stored at temp_float.
	 * This is because there is currently no way to report read_cb
	 * failures to the LWM2M engine.
	 */
	read_enviroment_sensor(die_dev, temp_unit, SENSOR_CHAN_AMBIENT_TEMP, &temp_float);
#endif
	lwm2m_engine_set_float32("3303/0/5700", &temp_float);
	*data_len = sizeof(temp_float);
	/* get current time from device */
	lwm2m_engine_get_s32("3/0/13", &ts);
	/* set timestamp */
	lwm2m_engine_set_s32("3303/0/5518", ts);

	return &temp_float;
}

static void *pressure_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	int32_t ts;

	/* Only object instance 0 is currently used */
	if (obj_inst_id != 0) {
		*data_len = 0;
		return NULL;
	}

#if defined(ENV_SENSOR_NAME)
	/*
	 * No need to check if read was successful, just reuse the
	 * previous value which is already stored at temp_float.
	 * This is because there is currently no way to report read_cb
	 * failures to the LWM2M engine.
	 */
	read_enviroment_sensor(die_dev, press_unit, SENSOR_CHAN_PRESS, &press_float);
#endif
	lwm2m_engine_set_float32("3323/0/5700", &press_float);
	*data_len = sizeof(press_float);
	/* get current time from device */
	lwm2m_engine_get_s32("3/0/13", &ts);
	/* set timestamp */
	lwm2m_engine_set_s32("3323/0/5518", ts);

	return &press_float;
}

static void *humidity_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	int32_t ts;

	/* Only object instance 0 is currently used */
	if (obj_inst_id != 0) {
		*data_len = 0;
		return NULL;
	}

#if defined(ENV_SENSOR_NAME)
	/*
	 * No need to check if read was successful, just reuse the
	 * previous value which is already stored at temp_float.
	 * This is because there is currently no way to report read_cb
	 * failures to the LWM2M engine.
	 */
	read_enviroment_sensor(die_dev, humid_unit, SENSOR_CHAN_HUMIDITY, &humid_float);
#endif
	lwm2m_engine_set_float32("3304/0/5700", &humid_float);
	*data_len = sizeof(humid_float);
	/* get current time from device */
	lwm2m_engine_get_s32("3/0/13", &ts);
	/* set timestamp */
	lwm2m_engine_set_s32("3304/0/5518", ts);

	return &humid_float;
}

int lwm2m_init_env_sensor(void)
{
#if defined(ENV_SENSOR_NAME)
	die_dev = device_get_binding(ENV_SENSOR_NAME);
	LOG_INF("%s on-die enviroment sensor %s",
		die_dev ? "Found" : "Did not find", ENV_SENSOR_NAME);
#endif

	if (!die_dev) {
		LOG_ERR("No enviroment sensor device found.");
	}

	lwm2m_engine_create_obj_inst("3303/0");
	lwm2m_engine_register_read_callback("3303/0/5700", temp_read_cb);
	lwm2m_engine_set_res_data("3303/0/5518",
				  &timestamp, sizeof(timestamp), 0);
	lwm2m_engine_set_res_data("3303/0/5701", temp_unit, sizeof(temp_unit),
				  LWM2M_RES_DATA_FLAG_RO);

    lwm2m_engine_create_obj_inst("3323/0");
	lwm2m_engine_register_read_callback("3323/0/5700", pressure_read_cb);
	lwm2m_engine_set_res_data("3323/0/5518",
				  &timestamp, sizeof(timestamp), 0);
	lwm2m_engine_set_res_data("3323/0/5701", press_unit, sizeof(press_unit),
				  LWM2M_RES_DATA_FLAG_RO);
	
    lwm2m_engine_create_obj_inst("3304/0");
	lwm2m_engine_register_read_callback("3304/0/5700", humidity_read_cb);
	lwm2m_engine_set_res_data("3304/0/5518",
				  &timestamp, sizeof(timestamp), 0);
	lwm2m_engine_set_res_data("3304/0/5701", humid_unit, sizeof(humid_unit),
				  LWM2M_RES_DATA_FLAG_RO);
	return 0;
}