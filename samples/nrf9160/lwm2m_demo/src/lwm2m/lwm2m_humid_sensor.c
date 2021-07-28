#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "sensor_event.h"
#include "env_sensor.h"

#define MODULE app_lwm2m_humid_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LWM2M_RES_DATA_FLAG_RW	0

#define MIN_RANGE_VALUE			0
#define MAX_RANGE_VALUE			100

#if defined(CONFIG_ENV_SENSOR_USE_EXTERNAL)
#define HUMID_APP_TYPE "BME680 Humidity Sensor"
#elif defined(CONFIG_ENV_SENSOR_USE_SIM)
#define HUMID_APP_TYPE "Simulated Humidity Sensor"
#endif

#define HUMID_UNIT 				"%"

static float32_value_t humid_float;
static bool read_sensor;

#if defined(CONFIG_LWM2M_IPSO_HUMIDITY_SENSOR_VERSION_1_1)
static int32_t timestamp;
static uint8_t meas_qual_ind;

static void set_timestamp(void)
{
	int32_t ts;

	lwm2m_engine_get_s32("3/0/13", &ts);
	lwm2m_engine_set_s32(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, TIMESTAMP_RID),
			ts);
}
#endif

static void *humidity_read_cb(uint16_t obj_inst_id, uint16_t res_id, 
					uint16_t res_inst_id, size_t *data_len)
{
	/* Only read sensor if a regular request from server, i.e. not a notify request */
	if (read_sensor) {
		int ret;
		struct sensor_value humid_val;

		ret = env_sensor_read_humidity(&humid_val);
		if (ret) {
			LOG_ERR("Error %d: read humidity sensor failed", ret);
			return NULL;
		}

#if defined(CONFIG_LWM2M_IPSO_HUMIDITY_SENSOR_VERSION_1_1)
		set_timestamp();
#endif

		humid_float.val1 = humid_val.val1;
		humid_float.val2 = humid_val.val2;
	}
	else {
		read_sensor = true;
	}

	*data_len = sizeof(humid_float);

	return &humid_float;
}

int lwm2m_init_humid_sensor(void)
{
	float32_value_t min_range_val = {
		.val1 = MIN_RANGE_VALUE, 
		.val2 = 0};
	float32_value_t max_range_val = {
		.val1 = MAX_RANGE_VALUE, 
		.val2 = 0};
	
	read_sensor = true;
	env_sensor_init();

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID), humidity_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID),
			&humid_float, sizeof(humid_float), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_UNITS_RID), 
			HUMID_UNIT, sizeof(HUMID_UNIT), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, MIN_RANGE_VALUE_RID),
			&min_range_val);
	lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, MAX_RANGE_VALUE_RID),
			&max_range_val);

#if defined(CONFIG_LWM2M_IPSO_HUMIDITY_SENSOR_VERSION_1_1)
	meas_qual_ind = 0;

	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, APPLICATION_TYPE_RID), 
			HUMID_APP_TYPE, sizeof(HUMID_APP_TYPE), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, TIMESTAMP_RID), 
			&timestamp, sizeof(timestamp), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, MEASUREMENT_QUALITY_INDICATOR_RID), 
			&meas_qual_ind, sizeof(meas_qual_ind), LWM2M_RES_DATA_FLAG_RW);
#endif

    return 0;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_sensor_event(eh)) {
		struct sensor_event *event = cast_sensor_event(eh);

        if (event->type == HumiditySensor) {
            float32_value_t received_value;

            /* This prevents re-reading the sensor when a callback is called because of
            a notification event.
            Ensures that the value received by the server is the same as the value in the
            event received below. */
            read_sensor = false;

            LOG_DBG("Humidity sensor event received: val1 = %06d, val2 = %06d", 
					event->sensor_value.val1, event->sensor_value.val2);

#if defined(CONFIG_LWM2M_IPSO_HUMIDITY_SENSOR_VERSION_1_1)
			set_timestamp();
#endif

			received_value.val1 = event->sensor_value.val1;
			received_value.val2 = event->sensor_value.val2;

			lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID),
				&received_value);

            return true;
        }
		
        return false;
	} 

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, sensor_event);