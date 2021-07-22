#include <zephyr.h>
#include <drivers/sensor.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>
#include <math.h>
#include <stdlib.h>

#include "sensor_event.h"
#include "env_sensor.h"
#include "light_sensor.h"

#define MODULE sensor_module

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define IPSO_OBJECT_COLOUR_ID 	3335
#define LIGHT_OBJ_INSTANCE_ID 	0
#define COLOUR_OBJ_INSTANCE_ID  1
#define RBG_STR_LEN				11	// '0xRRGGBBIR\0'

#if defined(CONFIG_SENSOR_MODULE_TEMP)

#define TEMP_STARTUP_TIME		K_SECONDS(CONFIG_SENSOR_MODULE_TEMP_STARTUP_DELAY)
#define TEMP_DELAY          	CONFIG_SENSOR_MODULE_TEMP_DELAY
#define TEMP_DELTA				((float32_value_t){ \
								.val1 = CONFIG_SENSOR_MODULE_TEMP_DELTA_INT, \
								.val2 = CONFIG_SENSOR_MODULE_TEMP_DELTA_DEC})
static struct k_work_delayable temp_work;
#endif

#if defined(CONFIG_SENSOR_MODULE_PRESS)
#define PRESS_STARTUP_TIME		K_SECONDS(CONFIG_SENSOR_MODULE_PRESS_STARTUP_DELAY)
#define PRESS_DELAY				CONFIG_SENSOR_MODULE_PRESS_DELAY
#define PRESS_DELTA				((float32_value_t){ \
								.val1 = CONFIG_SENSOR_MODULE_PRESS_DELTA_INT,\
								.val2 = CONFIG_SENSOR_MODULE_PRESS_DELTA_DEC})
static struct k_work_delayable press_work;
#endif

#if defined(CONFIG_SENSOR_MODULE_HUMID)
#define HUMID_STARTUP_TIME		K_SECONDS(CONFIG_SENSOR_MODULE_HUMID_STARTUP_DELAY)
#define HUMID_DELAY				CONFIG_SENSOR_MODULE_HUMID_DELAY
#define HUMID_DELTA				((float32_value_t){ \
								.val1 = CONFIG_SENSOR_MODULE_HUMID_DELTA_INT, \
								.val2 = CONFIG_SENSOR_MODULE_HUMID_DELTA_DEC})
static struct k_work_delayable humid_work;
#endif

#if defined(CONFIG_SENSOR_MODULE_GAS_RES)
#define GAS_RES_STARTUP_TIME	K_SECONDS(CONFIG_SENSOR_MODULE_GAS_RES_STARTUP_DELAY)
#define GAS_RES_DELAY			CONFIG_SENSOR_MODULE_GAS_RES_DELAY
#define GAS_RES_DELTA			((float32_value_t){ \
								.val1 = CONFIG_SENSOR_MODULE_GAS_RES_DELTA, \
								.val2 = 0})
static struct k_work_delayable gas_res_work;
#endif

#if defined(CONFIG_SENSOR_MODULE_LIGHT)
#define LIGHT_STARTUP_TIME		K_SECONDS(CONFIG_SENSOR_MODULE_LIGHT_STARTUP_DELAY)
#define LIGHT_DELAY				CONFIG_SENSOR_MODULE_LIGHT_DELAY
#define LIGHT_FETCH_DELAY_MS	(LIGHT_DELAY * MSEC_PER_SEC / 2)
#define LIGHT_DELTA			((uint32_t)((CONFIG_SENSOR_MODULE_LIGHT_DELTA_R << 24) | \
										(CONFIG_SENSOR_MODULE_LIGHT_DELTA_G << 16) | \
										(CONFIG_SENSOR_MODULE_LIGHT_DELTA_B << 8)  | \
										(CONFIG_SENSOR_MODULE_LIGHT_DELTA_IR)))
static struct k_work_delayable light_work;
#endif

#if defined(CONFIG_SENSOR_MODULE_COLOUR)
#define COLOUR_STARTUP_TIME		K_SECONDS(CONFIG_SENSOR_MODULE_COLOUR_STARTUP_DELAY)
#define COLOUR_DELAY			CONFIG_SENSOR_MODULE_COLOUR_DELAY
#define COLOUR_FETCH_DELAY_MS	(COLOUR_DELAY * MSEC_PER_SEC / 2)
#define COLOUR_DELTA		((uint32_t)((CONFIG_SENSOR_MODULE_COLOUR_DELTA_R << 24) | \
										(CONFIG_SENSOR_MODULE_COLOUR_DELTA_G << 16) | \
										(CONFIG_SENSOR_MODULE_COLOUR_DELTA_B << 8)  | \
										(CONFIG_SENSOR_MODULE_COLOUR_DELTA_IR)))
static struct k_work_delayable colour_work;
#endif

#if defined(CONFIG_SENSOR_MODULE_TEMP) || defined(CONFIG_SENSOR_MODULE_PRESS) || \
    defined(CONFIG_SENSOR_MODULE_HUMID) || defined(CONFIG_SENSOR_MODULE_GAS_RES)
static bool float32_sufficient_change(float32_value_t new_val, float32_value_t old_val, 
						float32_value_t req_change)
{
	int64_t int_change;
	float32_value_t change;

	int_change = (int64_t)(new_val.val1 - old_val.val1)*1000000 + 
				 (int64_t)(new_val.val2 - old_val.val2);

	change.val1 = fabs(int_change / 1000000);
	change.val2 = fabs(int_change % 1000000);

	if (change.val1 > req_change.val1) {
		return true;
	}
	else if (change.val1 == req_change.val1 && change.val2 >= req_change.val2) {
		return true;
	}
	return false;
}

static float32_value_t sensor_value_to_float32(struct sensor_value val)
{
	return (float32_value_t){.val1 = val.val1, .val2 = val.val2};
}
#endif /* if defined(CONFIG_SENSOR_MODULE_TEMP) || defined(CONFIG_SENSOR_MODULE_PRESS) || \
    		 defined(CONFIG_SENSOR_MODULE_HUMID) || defined(CONFIG_SENSOR_MODULE_GAS_RES) */

#if defined(CONFIG_SENSOR_MODULE_TEMP)
static void temp_work_cb(struct k_work *work) 
{
	float32_value_t *old_temp_val;
	uint16_t dummy_data_len;
	uint8_t dummy_data_flags;
	struct sensor_value new_temp_val;

	LOG_DBG("TEMP WORK CB");
	
	/* Get latest registered temperature value */
	lwm2m_engine_get_res_data(
		LWM2M_PATH(IPSO_OBJECT_TEMP_SENSOR_ID, 0, SENSOR_VALUE_RID),
		(void **)(&old_temp_val), &dummy_data_len, &dummy_data_flags);

	env_sensor_read_temperature(&new_temp_val);

	if (float32_sufficient_change(sensor_value_to_float32(new_temp_val), *old_temp_val, TEMP_DELTA)) {
		struct sensor_event *event = new_sensor_event();

		event->type = TemperatureSensor;
		event->sensor_value = new_temp_val;

		EVENT_SUBMIT(event);
	}

	k_work_schedule(&temp_work, K_SECONDS(TEMP_DELAY));
}
#endif /* if defined(CONFIG_SENSOR_MODULE_TEMP) */

#if defined(CONFIG_SENSOR_MODULE_PRESS)
static void press_work_cb(struct k_work *work)
{
	float32_value_t *old_press_val;
	uint16_t dummy_data_len;
	uint8_t dummy_data_flags;
	struct sensor_value new_press_val;

	LOG_DBG("PRESS WORK CB");

	/* Get latest registered pressure value */
	lwm2m_engine_get_res_data(
		LWM2M_PATH(IPSO_OBJECT_PRESSURE_ID, 0, SENSOR_VALUE_RID),
		(void **)(&old_press_val), &dummy_data_len, &dummy_data_flags);

	env_sensor_read_pressure(&new_press_val);

	if (float32_sufficient_change(sensor_value_to_float32(new_press_val), *old_press_val, PRESS_DELTA)) {
		struct sensor_event *event = new_sensor_event();

		event->type = PressureSensor;
		event->sensor_value = new_press_val;

		EVENT_SUBMIT(event);
	}

	k_work_schedule(&press_work, K_SECONDS(PRESS_DELAY));
}
#endif /* if defined(CONFIG_SENSOR_MODULE_PRESS) */

#if defined(CONFIG_SENSOR_MODULE_HUMID)
static void humid_work_cb(struct k_work *work)
{
	float32_value_t *old_humid_val;
	uint16_t dummy_data_len;
	uint8_t dummy_data_flags;
	struct sensor_value new_humid_val;

	LOG_DBG("HUMID WORK CB");

	/* Get latest registered humidity value */
	lwm2m_engine_get_res_data(
		LWM2M_PATH(IPSO_OBJECT_HUMIDITY_SENSOR_ID, 0, SENSOR_VALUE_RID),
		(void **)(&old_humid_val), &dummy_data_len, &dummy_data_flags);

	env_sensor_read_humidity(&new_humid_val);

	if (float32_sufficient_change(sensor_value_to_float32(new_humid_val), *old_humid_val, HUMID_DELTA)) {
		struct sensor_event *event = new_sensor_event();

		event->type = HumiditySensor;
		event->sensor_value = new_humid_val;

		EVENT_SUBMIT(event);
	}

	k_work_schedule(&humid_work, K_SECONDS(HUMID_DELAY));
}
#endif /* if defined(CONFIG_SENSOR_MODULE_HUMID) */

#if defined(CONFIG_SENSOR_MODULE_GAS_RES)
static void gas_res_work_cb(struct k_work *work)
{
	float32_value_t *old_gas_res_val;
	uint16_t dummy_data_len;
	uint8_t dummy_data_flags;
	struct sensor_value new_gas_res_val;

	LOG_DBG("GAS RES WORK CB");

	/* Get latest registered gas resistance value */
	lwm2m_engine_get_res_data(
		LWM2M_PATH(IPSO_OBJECT_GENERIC_SENSOR_ID, 0, SENSOR_VALUE_RID),
		(void **)(&old_gas_res_val), &dummy_data_len, &dummy_data_flags);

	env_sensor_read_gas_resistance(&new_gas_res_val);

	if (float32_sufficient_change(sensor_value_to_float32(new_gas_res_val), *old_gas_res_val, GAS_RES_DELTA)) {
		struct sensor_event *event = new_sensor_event();

		event->type = GasResistanceSensor;
		event->sensor_value = new_gas_res_val;

		EVENT_SUBMIT(event);
	}

	k_work_schedule(&gas_res_work, K_SECONDS(GAS_RES_DELAY));
}
#endif /* if defined(CONFIG_SENSOR_MODULE_GAS_RES) */

#if defined(CONFIG_SENSOR_MODULE_LIGHT) || defined(CONFIG_SENSOR_MODULE_COLOUR)
static bool rgbir_sufficient_change(uint32_t new_light_val, uint32_t old_light_val, uint32_t req_change)
{
	uint8_t *new_val_ptr = (uint8_t *)(&new_light_val);
	uint8_t *old_val_ptr = (uint8_t *)(&old_light_val);
	int16_t new_byte, old_byte;
	uint32_t change = 0;

	/* Get change per colour channel; each byte represents a unique colour channel */
	for (int i = 0; i < 4; i++) {
		new_byte = *(new_val_ptr + i);
		old_byte = *(old_val_ptr + i);

		change |= (uint32_t)(fabs(new_byte - old_byte)) << 8*i;
	}

	/* Check if any of the colour channels has changed sufficiently */
	for (int i = 0; i < 4; i++) {
		if ((uint8_t)(change >> 8*i) > (uint8_t)(req_change >> 8*i)) {
			return true;
		}
	}

	return false;
}
#endif /* if defined(CONFIG_SENSOR_MODULE_LIGHT) || defined(CONFIG_SENSOR_MODULE_COLOUR) */

#if defined(CONFIG_SENSOR_MODULE_LIGHT)
static void light_work_cb(struct k_work *work)
{
	char *old_light_val_str;
	uint16_t dummy_data_len;
	uint8_t dummy_data_flags;
	uint32_t old_light_val;
	uint32_t new_light_val;

	LOG_DBG("LIGHT WORK CB");

	/* Get latest registered light value */
	lwm2m_engine_get_res_data(
		LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, LIGHT_OBJ_INSTANCE_ID, COLOUR_RID),
		(void **)(&old_light_val_str), &dummy_data_len, &dummy_data_flags);
	old_light_val = strtol(old_light_val_str, NULL, 0);

	/* Read sensor, try again later if busy */
	if (light_sensor_read(&new_light_val) == -EBUSY) {
		k_work_schedule(&light_work, K_MSEC(LIGHT_FETCH_DELAY_MS));
		return;
	}

	if (rgbir_sufficient_change(new_light_val, old_light_val, LIGHT_DELTA)) {
		struct sensor_event *event = new_sensor_event();

		event->type = LightSensor;
		event->unsigned_value = new_light_val;

		EVENT_SUBMIT(event);
	}

	k_work_schedule(&light_work, K_SECONDS(LIGHT_DELAY));
}
#endif /* if defined(CONFIG_SENSOR_MODULE_LIGHT) */

#if defined(CONFIG_SENSOR_MODULE_COLOUR)
static void colour_work_cb(struct k_work *work)
{
	char *old_colour_val_str;
	uint16_t dummy_data_len;
	uint8_t dummy_data_flags;
	uint32_t old_colour_val;
	uint32_t new_colour_val;

	LOG_DBG("COLOUR WORK CB");

	/* Get latest registered colour value */
	lwm2m_engine_get_res_data(
		LWM2M_PATH(IPSO_OBJECT_COLOUR_ID, COLOUR_OBJ_INSTANCE_ID, COLOUR_RID),
		(void **)(&old_colour_val_str), &dummy_data_len, &dummy_data_flags);
	old_colour_val = strtol(old_colour_val_str, NULL, 0);

	/* Read sensor, try again later if busy */
	if (colour_sensor_read(&new_colour_val) == -EBUSY) {
		k_work_schedule(&colour_work, K_MSEC(COLOUR_FETCH_DELAY_MS));
		return;
	}

	if (rgbir_sufficient_change(new_colour_val, old_colour_val, COLOUR_DELTA)) {
		struct sensor_event *event = new_sensor_event();

		event->type = ColourSensor;
		event->unsigned_value = new_colour_val;

		EVENT_SUBMIT(event);
	}

	k_work_schedule(&colour_work, K_SECONDS(COLOUR_DELAY));
}
#endif /* if defined(CONFIG_SENSOR_MODULE_COLOUR) */

int sensor_module_init(void)
{
#if defined(CONFIG_SENSOR_MODULE_TEMP)
	k_work_init_delayable(&temp_work, temp_work_cb);
	k_work_schedule(&temp_work, TEMP_STARTUP_TIME);
#endif

#if defined(CONFIG_SENSOR_MODULE_PRESS)
	k_work_init_delayable(&press_work, press_work_cb);
	k_work_schedule(&press_work, PRESS_STARTUP_TIME);
#endif
	
#if defined(CONFIG_SENSOR_MODULE_HUMID)
	k_work_init_delayable(&humid_work, humid_work_cb);
	k_work_schedule(&humid_work, HUMID_STARTUP_TIME);
#endif

#if defined(CONFIG_SENSOR_MODULE_GAS_RES)
	k_work_init_delayable(&gas_res_work, gas_res_work_cb);
	k_work_schedule(&gas_res_work, GAS_RES_STARTUP_TIME);
#endif

#if defined(CONFIG_SENSOR_MODULE_LIGHT)
	k_work_init_delayable(&light_work, light_work_cb);
	k_work_schedule(&light_work, LIGHT_STARTUP_TIME);
#endif

#if defined(CONFIG_SENSOR_MODULE_COLOUR)
	k_work_init_delayable(&colour_work, colour_work_cb);
	k_work_schedule(&colour_work, COLOUR_STARTUP_TIME);
#endif

	return 0;
} 