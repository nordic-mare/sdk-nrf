#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>

#include "sensor_event.h"
#include "env_sensor.h"

#define MODULE app_lwm2m_press_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LWM2M_RES_DATA_FLAG_RW	0

#define PRESS_UNIT 		"kPa"

static bool read_sensor;
static float32_value_t press_float;

static void *pressure_read_cb(uint16_t obj_inst_id, uint16_t res_id, 
					uint16_t res_inst_id, size_t *data_len)
{
	/* Only read sensor if a regular request from server, i.e. not a notify request */
	if (read_sensor) {
		int ret;
		struct sensor_value press_val;

		ret = env_sensor_read_pressure(&press_val);
		if (ret) {
			LOG_ERR("Error %d: read air pressure sensor failed", ret);
			return NULL;
		}

		press_float.val1 = press_val.val1;
		press_float.val2 = press_val.val2;
	}
	else {
		read_sensor = true;
	}

	*data_len = sizeof(press_float);

	return &press_float;
}

int lwm2m_init_press_sensor(void)
{
	read_sensor = true;
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

    return 0;
}

static bool event_handler(const struct event_header *eh)
{
	if (is_sensor_event(eh)) {
		struct sensor_event *event = cast_sensor_event(eh);

        if (event->type == PressureSensor) {
            float32_value_t received_value;

            /* This prevents re-reading the sensor when a callback is called because of
            a notification event.
            Ensures that the value received by the server is the same as the value in the
            event received below. */
            read_sensor = false;

            LOG_DBG("Pressure sensor event received: val1 = %06d, val2 = %06d", 
					event->sensor_value.val1, event->sensor_value.val2);

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