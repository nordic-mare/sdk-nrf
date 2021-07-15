/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/gps.h>
#include <stdio.h>
#include <net/lwm2m.h>
#include <net/lwm2m_client_utils.h>
#include <net/lwm2m_path.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(lwm2m_app_loc, CONFIG_APP_LOG_LEVEL);

#define LATITUDE_RID 0
#define LONGITUDE_RID 1
#define ALTITUDE_RID 2
#define RADIUS_RID 3
#define SPEED_RID 6

static struct gps_pvt pvt_data;
static const struct device *gps_dev;
static struct gps_config gps_cfg = {
	.nav_mode = GPS_NAV_MODE_PERIODIC,
	.power_mode = GPS_POWER_MODE_DISABLED,
	.interval = CONFIG_GPS_SEARCH_INTERVAL_TIME,
	.timeout = CONFIG_GPS_SEARCH_TIMEOUT_TIME,
	.accuracy = GPS_ACCURACY_NORMAL,
	.priority = true
};

static float32_value_t float_to_lwm2m_float(float val) {
	float32_value_t out;
	out.val1 = (int32_t) val;
	out.val2 = (int32_t) ((out.val1 - val) * LWM2M_FLOAT32_DEC_MAX);
	return out;
}

static float32_value_t double_to_lwm2m_float(double val) {
	float32_value_t out;
	out.val1 = (int32_t) val;
	out.val2 = (int32_t) ((out.val1 - val) * LWM2M_FLOAT32_DEC_MAX);
	return out;
}

static void handle_pvt_fix(struct gps_pvt *pvt)
{
	float32_value_t latitude = double_to_lwm2m_float(pvt->latitude);
	float32_value_t longitude = double_to_lwm2m_float(pvt->longitude);
	float32_value_t altitude = float_to_lwm2m_float(pvt->altitude);
	float32_value_t speed = float_to_lwm2m_float(pvt->speed);
	float32_value_t radius = float_to_lwm2m_float(pvt->accuracy);

	lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, LATITUDE_RID), &latitude);
	lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, LONGITUDE_RID), &longitude);
	lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, ALTITUDE_RID), &altitude);
	lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, RADIUS_RID), &radius);
	lwm2m_engine_set_float32(LWM2M_PATH(LWM2M_OBJECT_LOCATION_ID, 0, SPEED_RID), &speed);
}

/**@brief Callback for GPS events */
static void gps_event_handler(const struct device *dev, struct gps_event *evt)
{
	ARG_UNUSED(dev);
	static uint32_t timestamp_prev;
	switch (evt->type)
	{
		case GPS_EVT_SEARCH_STARTED:
			LOG_DBG("GPS search started");
			break;
		case GPS_EVT_SEARCH_STOPPED:
			LOG_DBG("GPS search stopped");
			break;
		case GPS_EVT_SEARCH_TIMEOUT:
			LOG_DBG("GPS search timed out");
			break;
		case GPS_EVT_PVT:
			break;
		case GPS_EVT_PVT_FIX:
			if (k_uptime_get_32() - timestamp_prev < CONFIG_APP_HOLD_TIME_GPS * MSEC_PER_SEC) {
				break;
			}
			LOG_DBG("Recieved PVT Fix. GPS search completed.");
			memcpy(&pvt_data, &evt->pvt, sizeof(struct gps_pvt));
			handle_pvt_fix(&pvt_data);
			gps_cfg.priority = false;
			timestamp_prev = k_uptime_get_32();
			break;
		case GPS_EVT_NMEA:
			break;
		case GPS_EVT_NMEA_FIX:
			break;
		case GPS_EVT_OPERATION_BLOCKED:
			LOG_DBG("GPS search blocked");
			break;
		case GPS_EVT_OPERATION_UNBLOCKED:
			LOG_DBG("GPS unblocked. Resuming search.");
			break;
		case GPS_EVT_AGPS_DATA_NEEDED:
			LOG_DBG("GPS requests AGPS Data. AGPS is not implemented.");
			break;
		case GPS_EVT_ERROR:
			LOG_DBG("GPS error occured");
			break;	
	}
}

int lwm2m_app_init_location(void)
{
	int err;

	gps_dev = device_get_binding(CONFIG_GPS_DEV_NAME);
	if (gps_dev == NULL) {
		LOG_ERR("Could not get %s device",
			log_strdup(CONFIG_GPS_DEV_NAME));
		return -EINVAL;
	}

	LOG_DBG("GPS device found: %s", log_strdup(CONFIG_GPS_DEV_NAME));

	err = gps_init(gps_dev, gps_event_handler);
	if (err) {
		LOG_ERR("Could not set trigger, error code: %d", err);
		return err;
	}

	return 0;
}

int lwm2m_app_start_gps(void) 
{
	int err;
	err = gps_start(gps_dev, &gps_cfg);
	if (err) {
		LOG_ERR("Could not start GPS, error code : %d", err);
		return err;
	}
	LOG_DBG("Started GPS scan");
	return 0;
}
