#include <zephyr.h>
#include <drivers/sensor.h>
#include <stdio.h>

#include "ui_sense_led.h"

#define MODULE light_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LIGHT_SENSOR_NODE_ID	DT_PATH(soc, peripheral_40000000, i2c_a000, bh1749_38)
#define LIGHT_SENSOR_NAME	    DT_LABEL(LIGHT_SENSOR_NODE_ID)

/* Each channel measurement needs to be scaled to uint8_t */
#define LIGHT_MEAS_MAX_VAL              CONFIG_UI_LIGHT_SENSOR_MEASUREMENT_MAX_VALUE
#define COLOUR_MEAS_MAX_VAL             CONFIG_UI_COLOUR_SENSOR_MEASUREMENT_MAX_VALUE
#define SCALE_LIGHT_MEAS(raw_value)     (raw_value * 255 / LIGHT_MEAS_MAX_VAL)
#define SCALE_COLOUR_MEAS(raw_value)    (raw_value * 255 / COLOUR_MEAS_MAX_VAL)

/* Trigger values */
#ifdef CONFIG_UI_LIGHT_SENSOR_TRIGGER_ENABLE
#ifdef CONFIG_UI_LIGHT_SENSOR_TRIGGER_THRESH
#define THRESH_UPPER            50
#define THRESH_LOWER            0
#endif /* CONFIG_UI_LIGHT_SENSOR_TRIGGER_THRESH */
#define TRIGGER_CHANEL          SENSOR_CHAN_RED
#endif /* CONFIG_UI_LIGHT_SENSOR_TRIGGER_ENABLE */

#define NUM_COLOURS             4
#define NUM_BITS_PER_COLOUR     8
#define RGBIR_STR_LENGTH        11  // '0xRRGGBBIR\0'

#define SENSE_LED_ON_TIME_MS    500

static const struct device *light_sensor_dev;

static int sensor_read(uint32_t measurement_values[])
{
	int ret;
	struct sensor_value sensor_val;

	ret = sensor_sample_fetch_chan(light_sensor_dev, SENSOR_CHAN_ALL);
	if (ret) {
		LOG_ERR("Error %d: fetch sample failed", ret);
		return ret;
	}

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_RED, &sensor_val);
	if (ret) {
		LOG_ERR("Error %d: get red channel failed", ret);
		return ret;
	}
	measurement_values[0] = (uint32_t)sensor_val.val1;
	LOG_DBG("Light sensor raw red value: %i", sensor_val.val1);

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_GREEN, &sensor_val);
	if (ret) {
		LOG_ERR("Error %d: get green channel failed", ret);
		return ret;
	}
	measurement_values[1] = (uint32_t)sensor_val.val1;
	LOG_DBG("Light sensor raw green value: %i", sensor_val.val1);

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_BLUE, &sensor_val);
	if (ret) {
		LOG_ERR("Error %d: get blue channel failed", ret);
		return ret;
	}
	measurement_values[2] = (uint32_t)sensor_val.val1;
	LOG_DBG("Light sensor raw blue value: %i", sensor_val.val1);

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_IR, &sensor_val);
	if (ret) {
		LOG_ERR("Error %d: get IR channel failed", ret);
		return ret;
	}
	measurement_values[3] = (uint32_t)sensor_val.val1;
	LOG_DBG("Light sensor raw IR value: %i", sensor_val.val1);

	return 0;
}

#ifdef CONFIG_UI_LIGHT_SENSOR_TRIGGER_ENABLE
// TODO: Generate events when trigger fires
static void trigger_handler(const struct device *dev, struct sensor_trigger *trigger) 
{
	ARG_UNUSED(dev);
	switch (trigger->type)
	{
	case SENSOR_TRIG_THRESHOLD:
	{
		int ret; 

		LOG_DBG("Threshold trigger fired!");

		k_sleep(K_SECONDS(30));

		ret = sensor_sample_fetch_chan(light_sensor_dev, SENSOR_CHAN_ALL);
		if (ret) {
			LOG_ERR("Error %d: fetch sample failed", ret);
			return;
		}
		break;
	}

	case SENSOR_TRIG_DATA_READY:
	{
		int ret; 

		LOG_DBG("Data ready trigger fired!");

		k_sleep(K_SECONDS(30));

		ret = sensor_sample_fetch_chan(light_sensor_dev, SENSOR_CHAN_ALL);
		if (ret) {
			LOG_ERR("Error %d: fetch sample failed", ret);
		}
		break;
	}
	
	default:
		break;
	}
}
#endif

int light_sensor_read(uint32_t *measurement)
{
	int ret;
	uint32_t measurement_values[NUM_COLOURS];

	ret = sensor_read(measurement_values);
	if (ret) {
		LOG_ERR("Error %d: read light sensor failed", ret);
		return ret;
	}

	*measurement = 0;
	for (int i = 0; i < 4; ++i) {
		measurement_values[i] = SCALE_LIGHT_MEAS(measurement_values[i]);
		LOG_DBG("Scaled value: %d", measurement_values[i]);
		*measurement |= measurement_values[i] << (NUM_COLOURS - 1 - i)*NUM_BITS_PER_COLOUR;
	}
	
	LOG_DBG("Light value: 0x%08X", *measurement);

	return 0;
}

int colour_sensor_read(uint32_t *measurement)
{
	int ret;
	uint32_t measurement_values[NUM_COLOURS];

	ui_sense_led_on_off(true);
	k_sleep(K_MSEC(SENSE_LED_ON_TIME_MS));

	ret = sensor_read(measurement_values);
	if (ret) {
		LOG_ERR("Error %d: read colour sensor failed", ret);
		return ret;
	}

	ui_sense_led_on_off(false);

	*measurement = 0;
	for (int i = 0; i < NUM_COLOURS; ++i) {
		measurement_values[i] = SCALE_COLOUR_MEAS(measurement_values[i]);
		LOG_DBG("Scaled value: %d", measurement_values[i]);
		*measurement |= measurement_values[i] << (NUM_COLOURS -1 - i)*NUM_BITS_PER_COLOUR;
	}
	
	LOG_DBG("Light value: 0x%08X", *measurement);
	
	return 0;
}

int light_sensor_init(void)
{
	ui_sense_led_init();

	light_sensor_dev = device_get_binding(LIGHT_SENSOR_NAME);
	if (!light_sensor_dev) {
		LOG_ERR("Error %d: could not bind to Light Sensor device", -ENODEV);
		return -ENODEV;
	}

#ifdef CONFIG_UI_LIGHT_SENSOR_TRIGGER_ENABLE
	int ret;
	
#ifdef CONFIG_UI_LIGHT_SENSOR_TRIGGER_THRESH
	struct sensor_value temp_val;
	struct sensor_trigger sensor_trigger_config = {
		.type = SENSOR_TRIG_THRESHOLD,
		.chan = TRIGGER_CHANEL
	};

	temp_val.val1 = THRESH_UPPER;
	temp_val.val2 = 0;
	ret = sensor_attr_set(light_sensor_dev, SENSOR_CHAN_ALL,
						SENSOR_ATTR_UPPER_THRESH, &temp_val);
	if (ret) {
		LOG_ERR("Error %d: set upper threshold attribute failed", ret);
		return ret;
	}

	temp_val.val1 = THRESH_LOWER;
	ret = sensor_attr_set(light_sensor_dev, SENSOR_CHAN_ALL,
						SENSOR_ATTR_LOWER_THRESH, &temp_val);
	if (ret) {
		LOG_ERR("Error %d: set lower threshold attribute failed", ret);
		return ret;
	}
#else
	struct sensor_trigger sensor_trigger_config = {
		.type = SENSOR_TRIG_DATA_READY,
		.chan = TRIGGER_CHANEL
	};
#endif /* CONFIG_UI_LIGHT_SENSOR_TRIGGER_THRESH */
	
	ret = sensor_trigger_set(light_sensor_dev, &sensor_trigger_config,
					trigger_handler);
	if (ret) {
		LOG_ERR("Error %d: set trigger handler failed", ret);
		return ret;
	}
#endif /* CONFIG_UI_LIGHT_SENSOR_TRIGGER_ENABLE */

	return 0;
}