#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "sensor_event.h"
#include "env_sensor.h"

#define MODULE app_lwm2m_press_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LWM2M_RES_DATA_FLAG_RW	0

#define NOTIFICATION_REQUEST_DELAY_MS	500

#define MIN_RANGE_VALUE			0
#define MAX_RANGE_VALUE			100

#if defined(CONFIG_ENV_SENSOR_USE_EXTERNAL)
#define PRESS_APP_TYPE "BME680 Pressure Sensor"
#elif defined(CONFIG_ENV_SENSOR_USE_SIM)
#define PRESS_APP_TYPE "Simulated Pressure Sensor"
#endif

#define PRESS_UNIT 				"kPa"

static float32_value_t press_float;
static int64_t last_sensor_read_timestamp;

static bool is_regular_request(void)
{
	int64_t dt = k_uptime_get() - last_sensor_read_timestamp;
	return dt > NOTIFICATION_REQUEST_DELAY_MS;
}

#if defined(CONFIG_LWM2M_IPSO_PRESSURE_SENSOR_VERSION_1_1)
static int32_t timestamp;
static uint8_t meas_qual_ind;

static void set_timestamp(void)
{
	int32_t ts;

	lwm2m_engine_get_s32("3/0/13", &ts);
	lwm2m_engine_set_s32(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, TIMESTAMP_RID),
			ts);
}
#endif

static void *pressure_read_cb(uint16_t obj_inst_id, uint16_t res_id, 
					uint16_t res_inst_id, size_t *data_len)
{
	/* Only read sensor if a regular request from server, i.e. not a notify request */
	if (is_regular_request()) {
		int ret;
		struct sensor_value press_val;

		ret = env_sensor_read_pressure(&press_val);
		if (ret) {
			LOG_ERR("Error %d: read air pressure sensor failed", ret);
			return NULL;
		}

		last_sensor_read_timestamp = k_uptime_get();

#if defined(CONFIG_LWM2M_IPSO_PRESSURE_SENSOR_VERSION_1_1)
		set_timestamp();
#endif

		press_float.val1 = press_val.val1;
		press_float.val2 = press_val.val2;
	}

	*data_len = sizeof(press_float);

	return &press_float;
}

int lwm2m_init_press_sensor(void)
{
	float32_value_t min_range_val = {
		.val1 = MIN_RANGE_VALUE, 
		.val2 = 0};
	float32_value_t max_range_val = {
		.val1 = MAX_RANGE_VALUE, 
		.val2 = 0};

	last_sensor_read_timestamp = k_uptime_get();

	env_sensor_init();

	lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0));
	lwm2m_engine_register_read_callback(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID), pressure_read_cb);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID),
			&press_float, sizeof(press_float), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_UNITS_RID), 
			PRESS_UNIT, sizeof(PRESS_UNIT), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, APPLICATION_TYPE_RID), 
			PRESS_APP_TYPE, sizeof(PRESS_APP_TYPE), LWM2M_RES_DATA_FLAG_RO);
	lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, MIN_RANGE_VALUE_RID),
			&min_range_val);
	lwm2m_engine_set_float32(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, MAX_RANGE_VALUE_RID),
			&max_range_val);

#if defined(CONFIG_LWM2M_IPSO_PRESSURE_SENSOR_VERSION_1_1)
	meas_qual_ind = 0;

	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, TIMESTAMP_RID), 
			&timestamp, sizeof(timestamp), LWM2M_RES_DATA_FLAG_RW);
	lwm2m_engine_set_res_data(
			LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, MEASUREMENT_QUALITY_INDICATOR_RID), 
			&meas_qual_ind, sizeof(meas_qual_ind), LWM2M_RES_DATA_FLAG_RW);
#endif

    return 0;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_sensor_event(eh)) {
		struct sensor_event *event = cast_sensor_event(eh);

        if (event->type == PressureSensor) {
            float32_value_t received_value;

            last_sensor_read_timestamp = k_uptime_get();

            LOG_DBG("Pressure sensor event received: val1 = %06d, val2 = %06d", 
					event->sensor_value.val1, event->sensor_value.val2);

#if defined(CONFIG_LWM2M_IPSO_PRESSURE_SENSOR_VERSION_1_1)
			set_timestamp();
#endif

			received_value.val1 = event->sensor_value.val1;
			received_value.val2 = event->sensor_value.val2;

			lwm2m_engine_set_float32(
				LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID),
				&received_value);

            return true;
        }
		
        return false;
	} 

	return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, sensor_event);