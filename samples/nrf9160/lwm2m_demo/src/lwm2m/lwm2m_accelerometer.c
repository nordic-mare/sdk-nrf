/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <net/lwm2m.h>
#include <net/lwm2m_path.h>
#include <lwm2m_resource_ids.h>
#include <math.h>

#include "accelerometer.h"
#include "accel_event.h"

#define MODULE	app_lwm2m_accel

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define SENSOR_UNIT_NAME			"m/s2"

#define LWM2M_RES_DATA_FLAG_RW		0

static float32_value_t x_val;
static float32_value_t y_val;
static float32_value_t z_val;
uint8_t read_condition;

static void *accel_x_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	if (read_condition == 0) {
		int ret;
		struct accelerometer_sensor_data accel_data;

		ret = accelerometer_read(&accel_data);
		if (ret) {
			LOG_ERR("Error %d: read accelerometer failed", ret);
			return NULL;
		}

		x_val.val1 = accel_data.x.val1;
		x_val.val2 = accel_data.x.val2;
	}
	else {
		read_condition = MAX(0, read_condition - 1);
	}

	*data_len = sizeof(x_val);

	return &x_val;
}

static void *accel_y_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	if (read_condition == 0) {
		int ret;
		struct accelerometer_sensor_data accel_data;

		ret = accelerometer_read(&accel_data);
		if (ret) {
			LOG_ERR("Error %d: read accelerometer failed", ret);
			return NULL;
		}

		y_val.val1 = accel_data.y.val1;
		y_val.val2 = accel_data.y.val2;
	}
	else {
		read_condition = MAX(0, read_condition - 1);
	}

	*data_len = sizeof(y_val);

	return &y_val;
}

static void *accel_z_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	if (read_condition == 0) {
		int ret;
		struct accelerometer_sensor_data accel_data;

		ret = accelerometer_read(&accel_data);
		if (ret) {
			LOG_ERR("Error %d: read accelerometer failed", ret);
			return NULL;
		}

		z_val.val1 = accel_data.z.val1;
		z_val.val2 = accel_data.z.val2;
	}
	else {
		read_condition = MAX(0, read_condition - 1);
	}

	*data_len = sizeof(z_val);

	return &z_val;
}

int lwm2m_init_accel(void)
{
	read_condition = 0;
	accelerometer_init();

	/* create accel object */
	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0));
	lwm2m_engine_set_res_data(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, SENSOR_UNITS_RID),
				  SENSOR_UNIT_NAME, sizeof(SENSOR_UNIT_NAME),
				  LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_register_read_callback(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, X_VALUE_RID),
				  accel_x_read_cb);
	lwm2m_engine_register_read_callback(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Y_VALUE_RID),
				  accel_y_read_cb);
	lwm2m_engine_register_read_callback(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Z_VALUE_RID), 
				  accel_z_read_cb);
	lwm2m_engine_set_res_data(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, X_VALUE_RID),
				  &x_val, sizeof(x_val), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Y_VALUE_RID),
				  &y_val, sizeof(y_val), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Z_VALUE_RID),
				  &z_val, sizeof(z_val), LWM2M_RES_DATA_FLAG_RW);

	return 0;
}


static bool event_handler(const struct event_header *eh)
{
	if (is_accel_event(eh)) {
		struct accel_event *event = cast_accel_event(eh);
		float32_value_t received_value;

		/* This prevents re-reading the sensor when a callback is called because of
            a notification event.
            Ensures that the value received by the server is the same as the value in the
            event received below. */
        read_condition = 3;

		LOG_DBG("Accelerometer sensor event received: x = %d.%06d, y = %d.%06d, z = %d.%06d", 
					event->data.x.val1, event->data.x.val2, 
					event->data.y.val1, event->data.y.val2,
					event->data.z.val1, event->data.z.val2);

		received_value.val1 = event->data.x.val1;
		received_value.val2 = event->data.x.val2;
		lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, X_VALUE_RID),
			&received_value);

		received_value.val1 = event->data.y.val1;
		received_value.val2 = event->data.y.val2;
		lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Y_VALUE_RID),
			&received_value);

		received_value.val1 = event->data.z.val1;
		received_value.val2 = event->data.z.val2;
		lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Z_VALUE_RID),
			&received_value);


        return true;
	} 

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, accel_event);