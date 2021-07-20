#include <zephyr.h>
#include <drivers/gps.h>
#include <event_manager.h>

#include "app_gps_event.h"

#define MODULE gps_module

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

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
            struct app_gps_event *event = new_app_gps_event();
            event->pvt = pvt_data;
            EVENT_SUBMIT(event);
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

int initialise_gps(void)
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

int start_gps_search(void) 
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
