/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>
#include <stdio.h>

#include "sensor_event.h"
#include "light_sensor.h"

#define MODULE app_lwm2m_light_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LWM2M_RES_DATA_FLAG_RW	0

#define IPSO_OBJECT_COLOUR_ID 	3335
#define LIGHT_OBJ_INSTANCE_ID 	0
#define COLOUR_OBJ_INSTANCE_ID  1

#define RGBIR_STR_LENGTH      	11  // '0xRRGGBBIR\0'

#define SENSOR_FETCH_DELAY_MS	200

#define LIGHT_SENSOR_APP_NAME   "Light sensor"
#define COLOUR_SENSOR_APP_NAME  "Colour sensor"
#define LIGHT_UNIT              "RGB-IR"

static char light_value_str[RGBIR_STR_LENGTH] = "-";
static char colour_value_str[RGBIR_STR_LENGTH] = "-";
static bool read_sensor;

static void *light_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len) 
{
	/* Only read sensor if a regular request from server, i.e. not a notify request */
	if (read_sensor) {
		int ret;
		uint32_t light_value;
		
		ret = light_sensor_read(&light_value);
		/* Fetch failed. Wait before trying fetch again. */
		if (ret == -EBUSY) {
			k_sleep(K_MSEC(SENSOR_FETCH_DELAY_MS));
			ret = light_sensor_read(&light_value);
			if (ret) {
				LOG_ERR("Error %d: read light sensor failed", ret);
				return NULL;
			}	
		}
		else if (ret) {
			LOG_ERR("Error %d: read light sensor failed", ret);
			return NULL;
		}
		
		LOG_DBG("Light value: 0x%08X", light_value);

		snprintf(light_value_str, RGBIR_STR_LENGTH,    
					"0x%08X", light_value);
	} 
	else {
		read_sensor = true;
	}

	*data_len = sizeof(light_value_str);

	return &light_value_str;
}

static void *colour_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	/* Only read sensor if a regular request from server, i.e. not a notify request */
	if (read_sensor) {
		int ret;
		uint32_t colour_value ;

		ret = colour_sensor_read(&colour_value);
		/* Fetch failed. Wait before trying fetch again. */
		if (ret == -EBUSY) {
			k_sleep(K_MSEC(SENSOR_FETCH_DELAY_MS));
			ret = colour_sensor_read(&colour_value);
			if (ret) {
				LOG_ERR("Error %d: read colour sensor failed", ret);
				return NULL;
			}	
		}
		else if (ret) {
			LOG_ERR("Error %d: read colour sensor failed", ret);
			return NULL;
		}

		LOG_DBG("Colour value: 0x%08X", colour_value);

		snprintf(colour_value_str, RGBIR_STR_LENGTH,
					"0x%08X", colour_value);
	} 
	else {
		read_sensor = true;
	}

	*data_len = sizeof(colour_value_str);

	return &colour_value_str;
}

int lwm2m_init_light_sensor(void) 
{
	read_sensor = true;
	light_sensor_init();

	/* Ambient light sensor */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, LIGHT_OBJ_INSTANCE_ID));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, LIGHT_OBJ_INSTANCE_ID, COLOUR_RID), 
			light_sensor_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, LIGHT_OBJ_INSTANCE_ID, COLOUR_RID),
			&light_value_str, RGBIR_STR_LENGTH, LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, LIGHT_OBJ_INSTANCE_ID, APPLICATION_TYPE_RID),
			LIGHT_SENSOR_APP_NAME, 
			sizeof(LIGHT_SENSOR_APP_NAME),
			LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, LIGHT_OBJ_INSTANCE_ID, SENSOR_UNITS_RID), 
			LIGHT_UNIT, 
			sizeof(LIGHT_UNIT),
			LWM2M_RES_DATA_FLAG_RO);

	/* Surface colour sensor */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, COLOUR_OBJ_INSTANCE_ID));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, COLOUR_OBJ_INSTANCE_ID, COLOUR_RID), 
			colour_sensor_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, COLOUR_OBJ_INSTANCE_ID, COLOUR_RID),
			&colour_value_str, RGBIR_STR_LENGTH, LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, COLOUR_OBJ_INSTANCE_ID, APPLICATION_TYPE_RID),
			COLOUR_SENSOR_APP_NAME, 
			sizeof(COLOUR_SENSOR_APP_NAME),
			LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, COLOUR_OBJ_INSTANCE_ID, SENSOR_UNITS_RID), 
			LIGHT_UNIT, 
			sizeof(LIGHT_UNIT),
			LWM2M_RES_DATA_FLAG_RO);

	return 0;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_sensor_event(eh)) {
		struct sensor_event *event = cast_sensor_event(eh);
		char temp_value_str[RGBIR_STR_LENGTH];
		
		/* This prevents re-reading the sensor when a callback is called because of
		   a notification event.
		   Ensures that the value received by the server is the same as the value in the
		   event received below. */
		read_sensor = false;

		switch (event->type)
		{
		case LightSensor:
			snprintf(temp_value_str, RGBIR_STR_LENGTH,    
				"0x%08X", event->unsigned_value);
			LOG_DBG("Light sensor event received! Val: 0x%08X", event->unsigned_value);

			lwm2m_engine_set_string(
				LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, LIGHT_OBJ_INSTANCE_ID, COLOUR_RID),
				temp_value_str);
			break;
		
		case ColourSensor:
			snprintf(temp_value_str, RGBIR_STR_LENGTH,
				"0x%08X", event->unsigned_value);

			LOG_DBG("Light sensor event received! Val: 0x%08X", event->unsigned_value);

			lwm2m_engine_set_string(
				LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, COLOUR_OBJ_INSTANCE_ID, COLOUR_RID),
				temp_value_str);
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