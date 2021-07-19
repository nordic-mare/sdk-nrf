/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "sensor_event.h"
#include "env_sensor.h"

#define MODULE app_lwm2m_env_sens

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LWM2M_RES_DATA_FLAG_RW	0

#define GENERIC_SENSOR_APP_TYPE "A measure for Air Quality Index"
#define GENERIC_SENSOR_TYPE 	"Gas resistance sensor"

#define TEMP_UNIT 		"°C"
#define PRESS_UNIT 		"kPa"
#define HUMID_UNIT 		"%"
#define GAS_RES_UNIT 	"Ohm"
	
/* Default values used if no environment sensor present on board */
static float32_value_t temp_float = {25, 0};
static float32_value_t press_float = {101, 325000};
static float32_value_t humid_float = {42, 0};
static float32_value_t gas_res_float = {30000, 0};

#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
static bool read_sensor;
#endif

static void *temp_read_cb(uint16_t obj_inst_id, uint16_t res_id, 
					uint16_t res_inst_id, size_t *data_len)
{
#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
	int ret;
	struct sensor_value temp_val;

	LOG_DBG("TEMP READ CALLBACK!!!");

	if (read_sensor) {
		LOG_DBG("Reading sensor");

		ret = env_sensor_read_temp(&temp_val);
		if (ret) {
			LOG_ERR("Error %d: read temperature sensor failed", ret);
			return NULL;
		}

		temp_float.val1 = temp_val.val1;
		temp_float.val2 = temp_val.val2;
	}
	else {
		read_sensor = true;
	}
#endif /* if defined(CONFIG_BOARD_THINGY91_NRF9160NS) */

	*data_len = sizeof(temp_float);

	return &temp_float;
}

static void *pressure_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
	int ret;
	struct sensor_value press_val;

	LOG_DBG("PRESS READ CALLBACK!!!");

	if (read_sensor) {
		LOG_DBG("Reading sensor");

		ret = env_sensor_read_pressure(&press_val);
		if (ret) {
			LOG_ERR("Error %d: read pressure sensor failed", ret);
			return NULL;
		}

		press_float.val1 = press_val.val1;
		press_float.val2 = press_val.val2;
	}
	else {
		read_sensor = true;
	}
#endif /* if defined(CONFIG_BOARD_THINGY91_NRF9160NS) */

	*data_len = sizeof(press_float);

	return &press_float;
}

static void *humidity_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
	int ret;
	struct sensor_value humid_val;

	if (read_sensor) {
		ret = env_sensor_read_humidity(&humid_val);
		if (ret) {
			LOG_ERR("Error %d: read humidity sensor failed", ret);
			return NULL;
		}

		humid_float.val1 = humid_val.val1;
		humid_float.val2 = humid_val.val2;
	}
	else {
		read_sensor = true;
	}
#endif /* if defined(CONFIG_BOARD_THINGY91_NRF9160NS) */

	*data_len = sizeof(humid_float);

	return &humid_float;
}

static void *gas_resistance_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
	int ret;
	struct sensor_value gas_res_val;
	
	if (read_sensor) {
		ret = env_sensor_read_gas_resistance(&gas_res_val);
		if (ret) {
			LOG_ERR("Error %d: read gas resistance sensor failed", ret);
			return NULL;
		}

		gas_res_float.val1 = gas_res_val.val1;
		gas_res_float.val2 = gas_res_val.val2;
	}
	else {
		read_sensor = true;
	}
#endif /* if defined(CONFIG_BOARD_THINGY91_NRF9160NS) */

	*data_len = sizeof(gas_res_float);

	return &gas_res_float;
}

int lwm2m_init_env_sensor(void)
{
#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
	read_sensor = true;
	env_sensor_init();
#endif

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0, SENSOR_VALUE_RID), temp_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0, SENSOR_VALUE_RID),
			&temp_float, sizeof(temp_float), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0, SENSOR_UNITS_RID), 
			TEMP_UNIT, sizeof(TEMP_UNIT), LWM2M_RES_DATA_FLAG_RO);

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID), pressure_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID),
			&press_float, sizeof(press_float), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_UNITS_RID), 
			PRESS_UNIT, sizeof(PRESS_UNIT), LWM2M_RES_DATA_FLAG_RO);
	
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID), humidity_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID),
			&humid_float, sizeof(humid_float), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_UNITS_RID), 
			HUMID_UNIT, sizeof(HUMID_UNIT), LWM2M_RES_DATA_FLAG_RO);

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, SENSOR_VALUE_RID), gas_resistance_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, SENSOR_VALUE_RID),
			&gas_res_float, sizeof(gas_res_float), LWM2M_RES_DATA_FLAG_RW);
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

#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
static bool event_handler(const struct event_header *eh)
{
	if (is_sensor_event(eh)) {
		struct sensor_event *event = cast_sensor_event(eh);
		float32_value_t received_value;
		read_sensor = false;

		switch (event->type)
		{
		case TemperatureSensor:
			LOG_DBG("Temperature sensor event received: val1 = %06d, val2 = %06d", 
					event->sensor_value.val1, event->sensor_value.val2);

			received_value.val1 = event->sensor_value.val1;
			received_value.val2 = event->sensor_value.val2;

			lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0, SENSOR_VALUE_RID),
				&received_value);
			break;

		case PressureSensor:
			LOG_DBG("Pressure sensor event received: val1 = %06d, val2 = %06d", 
					event->sensor_value.val1, event->sensor_value.val2);

			received_value.val1 = event->sensor_value.val1;
			received_value.val2 = event->sensor_value.val2;

			lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID),
				&received_value);
			break;

		case HumiditySensor:
			LOG_DBG("Humidity sensor event received! val1: val1 = %06d, val2 = %06d", 
					event->sensor_value.val1, event->sensor_value.val2);

			received_value.val1 = event->sensor_value.val1;
			received_value.val2 = event->sensor_value.val2;

			lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID),
				&received_value);
			break;

		case GasResistanceSensor:
			LOG_DBG("Gas resistance sensor event received! val1: val1 = %06d, val2 = %06d", 
					event->sensor_value.val1, event->sensor_value.val2);

			received_value.val1 = event->sensor_value.val1;
			received_value.val2 = event->sensor_value.val2;

			lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, SENSOR_VALUE_RID),
				&received_value);
			break;

		default:
			read_sensor = true;
			return false;
		} 

		return true;
	} 

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, sensor_event);
#endif /* if defined(CONFIG_BOARD_THINGY91_NRF9160NS) */