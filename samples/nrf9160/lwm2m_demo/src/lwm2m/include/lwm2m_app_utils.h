/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <drivers/sensor.h>

#ifndef LWM2M_APP_UTILS_H__
#define LWM2M_APP_UTILS_H__

/* LwM2M IPSO Object IDs */
#define IPSO_OBJECT_SECURITY_ID 0
#define IPSO_OBJECT_SERVER_ID 1

#define IPSO_OBJECT_DEVICE_ID 3

#define IPSO_OBJECT_COLOUR_ID 3335

/* Server RIDs */
#define LIFETIME_RID 1

/* Device RIDs */
#define MANUFACTURER_RID 0
#define MODEL_NUMBER_RID 1
#define SERIAL_NUMBER_RID 2
#define FACTORY_RESET_RID 5
#define POWER_SOURCE_RID 6
#define POWER_SOURCE_VOLTAGE_RID 7
#define POWER_SOURCE_CURRENT_RID 8
#define CURRENT_TIME_RID 13
#define DEVICE_TYPE_RID 17
#define HARDWARE_VERSION_RID 18
#define BATTERY_STATUS_RID 20
#define MEMORY_TOTAL_RID 21

/* Location RIDs */
#define LATITUDE_RID 0
#define LONGITUDE_RID 1
#define ALTITUDE_RID 2
#define LOCATION_RADIUS_RID 3
#define LOCATION_VELOCITY_RID 4
#define LOCATION_TIMESTAMP_RID 5
#define LOCATION_SPEED_RID 6

/* Misc */
#define LWM2M_RES_DATA_FLAG_RW 0
#define MAX_LWM2M_PATH_LEN 20
#define RGBIR_STR_LENGTH 11 /*'0xRRGGBBIR\0' */

#ifdef __cplusplus
extern "C" {
#endif

/* Set timestamp resource */
void lwm2m_set_timestamp(int ipso_obj_id, unsigned int obj_inst_id);

/* Check whether notification read callback or regular read callback */
bool is_regular_read_cb(int64_t read_timestamp);

float32_value_t double_to_float32(double val);
float32_value_t sensor_value_to_float32(struct sensor_value val);

double float32_to_double(float32_value_t val);

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_APP_UTILS_H__ */
