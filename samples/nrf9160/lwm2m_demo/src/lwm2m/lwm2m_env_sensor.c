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

#define TEMP_UNIT "C"
#define PRESS_UNIT "kPa"
#define HUMID_UNIT "%"
#define GAS_RES_UNIT "Ohm"

static float32_value_t temp_float;
static float32_value_t press_float;
static float32_value_t humid_float;
static float32_value_t gas_resistance_float;	

static const struct device *die_dev;

static int read_enviroment_sensor(const struct device *sens_dev, char *unit,
                enum sensor_channel channel, float32_value_t *float_val)
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

static void *temp_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	
	read_enviroment_sensor(die_dev, TEMP_UNIT, SENSOR_CHAN_AMBIENT_TEMP, &temp_float);

	*data_len = sizeof(temp_float);

	return &temp_float;
}

static void *pressure_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{

	read_enviroment_sensor(die_dev, PRESS_UNIT, SENSOR_CHAN_PRESS, &press_float);

	*data_len = sizeof(press_float);

	return &press_float;
}

static void *humidity_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{

	read_enviroment_sensor(die_dev, HUMID_UNIT, SENSOR_CHAN_HUMIDITY, &humid_float);

	*data_len = sizeof(humid_float);

	return &humid_float;
}

static void *gas_resistance_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	read_enviroment_sensor(die_dev, GAS_RES_UNIT, SENSOR_CHAN_GAS_RES, &gas_resistance_float);

	*data_len = sizeof(gas_resistance_float);

	return &gas_resistance_float;
}

int lwm2m_init_env_sensor(void)
{
	die_dev = device_get_binding(ENV_SENSOR_NAME);
	LOG_INF("%s on-die enviroment sensor %s",
		die_dev ? "Found" : "Did not find", ENV_SENSOR_NAME);

	if (!die_dev) {
		LOG_ERR("No enviroment sensor device found.");
	}

	lwm2m_engine_create_obj_inst("3303/0");
	lwm2m_engine_register_read_callback("3303/0/5700", temp_read_cb);
	lwm2m_engine_set_res_data("3303/0/5701", TEMP_UNIT, sizeof(TEMP_UNIT),
				  LWM2M_RES_DATA_FLAG_RO);

    lwm2m_engine_create_obj_inst("3323/0");
	lwm2m_engine_register_read_callback("3323/0/5700", pressure_read_cb);
	lwm2m_engine_set_res_data("3323/0/5701", PRESS_UNIT, sizeof(PRESS_UNIT),
				  LWM2M_RES_DATA_FLAG_RO);
	
    lwm2m_engine_create_obj_inst("3304/0");
	lwm2m_engine_register_read_callback("3304/0/5700", humidity_read_cb);
	lwm2m_engine_set_res_data("3304/0/5701", HUMID_UNIT, sizeof(HUMID_UNIT),
				  LWM2M_RES_DATA_FLAG_RO);

	lwm2m_engine_create_obj_inst("3300/0");
	lwm2m_engine_register_read_callback("3300/0/5700", gas_resistance_read_cb);
	lwm2m_engine_set_res_data("3300/0/5701", GAS_RES_UNIT, sizeof(GAS_RES_UNIT),
				  LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data("3300/0/5750",
                "A measure for air quality", 
				sizeof("A measure for air quality"),
                LWM2M_RES_DATA_FLAG_RO);
	return 0;
}