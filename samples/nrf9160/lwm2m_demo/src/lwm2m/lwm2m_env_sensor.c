/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <net/lwm2m.h>

#include "ui_env_sensor.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_env_sens, CONFIG_APP_LOG_LEVEL);

#define TEMP_UNIT "C"
#define PRESS_UNIT "kPa"
#define HUMID_UNIT "%"
#define GAS_RES_UNIT "Ohm"

static float32_value_t temp_float;
static float32_value_t press_float;
static float32_value_t humid_float;
static float32_value_t gas_resistance_float;	


static void *temp_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_read_env_sensor(TEMP_UNIT, SENSOR_CHAN_AMBIENT_TEMP, &temp_float);	

	*data_len = sizeof(temp_float);

	return &temp_float;
}

static void *pressure_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_read_env_sensor(PRESS_UNIT, SENSOR_CHAN_PRESS, &press_float);

	*data_len = sizeof(press_float);

	return &press_float;
}

static void *humidity_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_read_env_sensor(HUMID_UNIT, SENSOR_CHAN_HUMIDITY, &humid_float);

	*data_len = sizeof(humid_float);

	return &humid_float;
}

static void *gas_resistance_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_read_env_sensor(GAS_RES_UNIT, SENSOR_CHAN_GAS_RES, &gas_resistance_float);

	*data_len = sizeof(gas_resistance_float);

	return &gas_resistance_float;
}

int lwm2m_init_env_sensor(void)
{
	ui_env_sensor_init();

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