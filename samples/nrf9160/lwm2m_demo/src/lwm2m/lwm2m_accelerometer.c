/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>
#include <math.h>

#include "accelerometer.h"
#include "accel_event.h"

#define MODULE	app_lwm2m_accel

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#if defined(CONFIG_ACCEL_USE_EXTERNAL)
#define ACCEL_APP_TYPE			"ADXL362 Accelerometer"
#elif defined(CONFIG_ACCEL_USE_SIM)
#define ACCEL_APP_TYPE			"Simulated Accelerometer"
#endif

#define SENSOR_UNIT_NAME		"m/s^2"

#define LWM2M_RES_DATA_FLAG_RW	0

#define NOTIFICATION_REQUEST_DELAY_MS	1500

#if defined(CONFIG_ADXL362_ACCEL_RANGE_8G)
#define ACCEL_RANGE_G			8
#elif defined(CONFIG_ADXL362_ACCEL_RANGE_4G)
#define ACCEL_RANGE_G			4
#else
#define ACCEL_RANGE_G			2
#endif

#define MIN_RANGE_VALUE		(-ACCEL_RANGE_G*SENSOR_G/1000000.0)
#define MAX_RANGE_VALUE		(ACCEL_RANGE_G*SENSOR_G/1000000.0)

static float32_value_t *x_val;
static float32_value_t *y_val;
static float32_value_t *z_val;
static int64_t accel_read_timestamp[3];

static bool is_regular_request(uint16_t res_inst_id)
{
	int64_t dt = 0;
	
	switch (res_inst_id)
	{
	case X_VALUE_RID:
		dt = k_uptime_get() - accel_read_timestamp[0];
		break;

	case Y_VALUE_RID:
		dt = k_uptime_get() - accel_read_timestamp[1];
		break;

	case Z_VALUE_RID:
		dt = k_uptime_get() - accel_read_timestamp[2];
		break;
	
	default:
		break;
	}
	
	return dt > NOTIFICATION_REQUEST_DELAY_MS;
}

#if defined(CONFIG_LWM2M_IPSO_ACCELEROMETER_VERSION_1_1)
int32_t timestamp;
static uint8_t meas_qual_ind;

static void set_timestamp(void)
{
	int32_t ts;

	lwm2m_engine_get_s32("3/0/13", &ts);
	lwm2m_engine_set_s32(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, TIMESTAMP_RID),
			ts);	
}
#endif

static void *accel_x_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	if (is_regular_request(res_inst_id)) {
		int ret;
		struct accelerometer_sensor_data accel_data;
		float32_value_t new_x_val;

		ret = accelerometer_read(&accel_data);
		if (ret) {
			LOG_ERR("Error %d: read accelerometer failed", ret);
			return NULL;
		}

		accel_read_timestamp[0] = k_uptime_get();

#if defined(CONFIG_LWM2M_IPSO_ACCELEROMETER_VERSION_1_1)
		set_timestamp();
#endif

		new_x_val.val1 = accel_data.x.val1;
		new_x_val.val2 = accel_data.x.val2;

		lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, X_VALUE_RID),
				&new_x_val);
	}

	*data_len = sizeof(*x_val);

	return x_val;
}

static void *accel_y_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	if (is_regular_request(res_inst_id)) {
		int ret;
		struct accelerometer_sensor_data accel_data;
		float32_value_t new_y_val;

		ret = accelerometer_read(&accel_data);
		if (ret) {
			LOG_ERR("Error %d: read accelerometer failed", ret);
			return NULL;
		}

		accel_read_timestamp[1] = k_uptime_get();

#if defined(CONFIG_LWM2M_IPSO_ACCELEROMETER_VERSION_1_1)
		set_timestamp();
#endif

		new_y_val.val1 = accel_data.y.val1;
		new_y_val.val2 = accel_data.y.val2;

		lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Y_VALUE_RID),
				&new_y_val);
	}

	*data_len = sizeof(*y_val);

	return y_val;
}

static void *accel_z_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
	if (is_regular_request(res_inst_id)) {
		int ret;
		struct accelerometer_sensor_data accel_data;
		float32_value_t new_z_val;

		ret = accelerometer_read(&accel_data);
		if (ret) {
			LOG_ERR("Error %d: read accelerometer failed", ret);
			return NULL;
		}

		accel_read_timestamp[2] = k_uptime_get();

#if defined(CONFIG_LWM2M_IPSO_ACCELEROMETER_VERSION_1_1)
		set_timestamp();
#endif

		new_z_val.val1 = accel_data.z.val1;
		new_z_val.val2 = accel_data.z.val2;

		lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Z_VALUE_RID),
				&new_z_val);
	}

	*data_len = sizeof(*z_val);

	return z_val;
}

int lwm2m_init_accel(void)
{
	float32_value_t min_range_val = {
		.val1 = (int)MIN_RANGE_VALUE, 
		.val2 = (MIN_RANGE_VALUE - (int)MIN_RANGE_VALUE)*100000};
	float32_value_t max_range_val = {
		.val1 = (int)MAX_RANGE_VALUE, 
		.val2 = (MAX_RANGE_VALUE - (int)MAX_RANGE_VALUE)*100000};
	uint16_t dummy_data_len;
	uint8_t dummy_data_flags;

	accel_read_timestamp[0] = k_uptime_get();
	accel_read_timestamp[1] = k_uptime_get();
	accel_read_timestamp[2] = k_uptime_get();

	accelerometer_init();

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0));
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, SENSOR_UNITS_RID),
			SENSOR_UNIT_NAME, sizeof(SENSOR_UNIT_NAME),
			LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, X_VALUE_RID),
			accel_x_read_cb);
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Y_VALUE_RID),
			accel_y_read_cb);
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Z_VALUE_RID), 
			accel_z_read_cb);
	lwm2m_engine_get_res_data(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, X_VALUE_RID),
			(void **)&x_val, &dummy_data_len, &dummy_data_flags);
	lwm2m_engine_get_res_data(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Y_VALUE_RID),
			(void **)&y_val, &dummy_data_len, &dummy_data_flags);
	lwm2m_engine_get_res_data(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, Z_VALUE_RID),
			(void **)&z_val, &dummy_data_len, &dummy_data_flags);
	lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, MIN_RANGE_VALUE_RID),
			&min_range_val);
	lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, MAX_RANGE_VALUE_RID),
			&max_range_val);

#if defined(CONFIG_LWM2M_IPSO_ACCELEROMETER_VERSION_1_1)
	meas_qual_ind = 0;

	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, APPLICATION_TYPE_RID), 
			ACCEL_APP_TYPE, sizeof(ACCEL_APP_TYPE), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, TIMESTAMP_RID), 
			&timestamp, sizeof(timestamp), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_ACCELEROMETER_ID, 0, MEASUREMENT_QUALITY_INDICATOR_RID), 
			&meas_qual_ind, sizeof(meas_qual_ind), LWM2M_RES_DATA_FLAG_RW);
#endif

	return 0;
}


static bool event_handler(const struct event_header *eh)
{
	if (is_accel_event(eh)) {
		struct accel_event *event = cast_accel_event(eh);
		float32_value_t received_value;

		accel_read_timestamp[0] = k_uptime_get();
		accel_read_timestamp[1] = k_uptime_get();
		accel_read_timestamp[2] = k_uptime_get();

#if defined(CONFIG_LWM2M_IPSO_ACCELEROMETER_VERSION_1_1)
		set_timestamp();
#endif

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