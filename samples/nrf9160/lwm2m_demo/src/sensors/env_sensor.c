/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <stdlib.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(env_sensor, CONFIG_APP_LOG_LEVEL);

#if defined(CONFIG_ENV_SENSOR_USE_EXTERNAL)
#define ENV_SENSOR_NODE DT_PATH(soc, peripheral_40000000, i2c_a000, bme680_76)
#define ENV_SENSOR_DEV_LABEL DT_LABEL(ENV_SENSOR_NODE)
#elif defined(CONFIG_ENV_SENSOR_USE_SIM)
#define ENV_SENSOR_DEV_LABEL "SENSOR_SIM"
#define GAS_RES_SIM_BASE CONFIG_ENV_SENSOR_GAS_RES_SIM_BASE
#define GAS_RES_SIM_MAX_DIFF CONFIG_ENV_SENSOR_GAS_RES_SIM_MAX_DIFF
#endif

static const struct device *env_sensor_dev;

bool initialised;

static int read_sensor(struct sensor_value *value, enum sensor_channel channel)
{
	int ret;

	ret = sensor_sample_fetch(env_sensor_dev);
	if (ret) {
		LOG_ERR("Error %d: fetch sample failed", ret);
		return ret;
	}

	ret = sensor_channel_get(env_sensor_dev, channel, value);
	if (ret) {
		LOG_ERR("Error %d: get channel failed", ret);
		return ret;
	}

	return 0;
}

int env_sensor_read_temperature(struct sensor_value *temp_val)
{
	int ret;

	ret = read_sensor(temp_val, SENSOR_CHAN_AMBIENT_TEMP);
	if (ret) {
		LOG_ERR("Error %d: read temperatur sensor failed", ret);
		return ret;
	}

	LOG_INF("%s: read %d.%d °C", env_sensor_dev->name, temp_val->val1, temp_val->val2);

	return 0;
}

int env_sensor_read_pressure(struct sensor_value *press_value)
{
	int ret;

	ret = read_sensor(press_value, SENSOR_CHAN_PRESS);
	if (ret) {
		LOG_ERR("Error %d: read pressure sensor failed", ret);
		return ret;
	}

	LOG_INF("%s: read %d.%d kPa", env_sensor_dev->name, press_value->val1, press_value->val2);

	return 0;
}

int env_sensor_read_humidity(struct sensor_value *humid_val)
{
	int ret;

	ret = read_sensor(humid_val, SENSOR_CHAN_HUMIDITY);
	if (ret) {
		LOG_ERR("Error %d: read humidity sensor failed", ret);
		return ret;
	}

	LOG_INF("%s: read %d.%d %%", env_sensor_dev->name, humid_val->val1, humid_val->val2);

	return 0;
}

int env_sensor_read_gas_resistance(struct sensor_value *gas_res_val)
{
#if defined(CONFIG_ENV_SENSOR_USE_EXTERNAL)
	int ret;

	ret = read_sensor(gas_res_val, SENSOR_CHAN_GAS_RES);
	if (ret) {
		LOG_ERR("Error %d: read gas resistance sensor failed", ret);
		return ret;
	}
#elif defined(CONFIG_ENV_SENSOR_USE_SIM)
	int32_t sim_val =
		MAX(0, GAS_RES_SIM_BASE + (rand() % GAS_RES_SIM_MAX_DIFF) * (1 - 2 * (rand() % 2)));

	gas_res_val->val1 = sim_val;
	gas_res_val->val2 = 0;
#endif

	LOG_INF("%s: read %d.%d Ω", env_sensor_dev->name, gas_res_val->val1, gas_res_val->val2);

	return 0;
}

int env_sensor_init(void)
{
	if (!initialised) {
		env_sensor_dev = device_get_binding(ENV_SENSOR_DEV_LABEL);
		if (!env_sensor_dev) {
			LOG_ERR("Error %d: could not bind to Environment Sensor device", -ENODEV);
			return -ENODEV;
		}

		initialised = true;
	}

	return 0;
}
