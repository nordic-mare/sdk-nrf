/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "ui_env_sensor.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_env_sens, CONFIG_APP_LOG_LEVEL);

#define GENERIC_SENSOR_APP_TYPE "A measure for Air Quality Index"
#define GENERIC_SENSOR_TYPE 	"Gas resistance sensor"

#define TEMP_UNIT 		"C"
#define PRESS_UNIT 		"kPa"
#define HUMID_UNIT 		"%"
#define GAS_RES_UNIT 	"Ohm"

static float32_value_t temp_float;
static float32_value_t press_float;
static float32_value_t humid_float;
static float32_value_t gas_res_float;	


static void *temp_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_env_sensor_read_temp(&(temp_float.val1), &(temp_float.val2));

	*data_len = sizeof(temp_float);

	return &temp_float;
}

static void *pressure_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_env_sensor_read_pressure(&(press_float.val1), &(press_float.val2));

	*data_len = sizeof(press_float);

	return &press_float;
}

static void *humidity_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_env_sensor_read_humidity(&(humid_float.val1), &(humid_float.val2));

	*data_len = sizeof(humid_float);

	return &humid_float;
}

static void *gas_resistance_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	ui_env_sensor_read_gas_resistance(&(gas_res_float.val1), &(gas_res_float.val2));

	*data_len = sizeof(gas_res_float);

	return &gas_res_float;
}

int lwm2m_init_env_sensor(void)
{
	ui_env_sensor_init();

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0, SENSOR_VALUE_RID), temp_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0, SENSOR_UNITS_RID), 
			TEMP_UNIT, sizeof(TEMP_UNIT), LWM2M_RES_DATA_FLAG_RO);

    lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID), pressure_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_UNITS_RID), 
			PRESS_UNIT, sizeof(PRESS_UNIT), LWM2M_RES_DATA_FLAG_RO);
	
    lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID), humidity_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_UNITS_RID), 
			HUMID_UNIT, sizeof(HUMID_UNIT), LWM2M_RES_DATA_FLAG_RO);

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, SENSOR_VALUE_RID), gas_resistance_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, SENSOR_UNITS_RID), 
			GAS_RES_UNIT, sizeof(GAS_RES_UNIT), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, APPLICATION_TYPE_RID),
            GENERIC_SENSOR_APP_TYPE, sizeof(GENERIC_SENSOR_APP_TYPE), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, SENSOR_TYPE_RID),
			GENERIC_SENSOR_TYPE, sizeof(GENERIC_SENSOR_TYPE), LWM2M_RES_DATA_FLAG_RO);
	return 0;
}