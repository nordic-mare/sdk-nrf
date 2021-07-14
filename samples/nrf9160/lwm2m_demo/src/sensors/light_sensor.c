#include <zephyr.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <math.h>

#include "ui_sense_led.h"

#define MODULE light_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LIGHT_SENSOR_NODE_ID	DT_PATH(soc, peripheral_40000000, i2c_a000, bh1749_38)
#define LIGHT_SENSOR_NAME	    DT_LABEL(LIGHT_SENSOR_NODE_ID)

/* Trigger values */
#ifdef CONFIG_LIGHT_SENSOR_TRIGGER_ENABLE
#define TRIGGER_CHANEL          CONFIG_LIGHT_SENSOR_TRIGGER_THRESH_CHANNEL
#ifdef CONFIG_LIGHT_SENSOR_TRIGGER_THRESH
#define THRESH_UPPER            CONFIG_LIGHT_SENSOR_TRIGGER_THRESH_UPPER
#define THRESH_LOWER            CONFIG_LIGHT_SENSOR_TRIGGER_THRESH_LOWER
#endif /* CONFIG_LIGHT_SENSOR_TRIGGER_THRESH */
#endif /* CONFIG_LIGHT_SENSOR_TRIGGER_ENABLE */

#define RGBIR_STR_LENGTH        11U  /* Format: '0xRRGGBBIR\0'. */

#define SENSOR_FETCH_DELAY_MS	200U /* Time before a new fetch can be tried. */
#define SENSE_LED_ON_TIME_MS    300U /* Time the sense LED stays on during colour read. */

static const struct device *light_sensor_dev;
static int64_t fetch_timestamp;

/* sensor_sample_fetch_chan fails if called multiple times in a row with no delay. */
static bool fetch_ready()
{
	return k_uptime_get() > fetch_timestamp + SENSOR_FETCH_DELAY_MS;
}

static int sensor_read(struct sensor_value measurements[])
{
	int ret;

	if (fetch_ready()) {
		ret = sensor_sample_fetch_chan(light_sensor_dev, SENSOR_CHAN_ALL);
		if (ret) {
			LOG_ERR("Error %d: fetch sample failed", ret);
			return ret;
		}
		fetch_timestamp = k_uptime_get();
	}
	else {
		LOG_DBG("Sensor fetch not ready");
		return -EBUSY;
	}

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_IR, &measurements[0]);
	if (ret) {
		LOG_ERR("Error %d: get IR channel failed", ret);
		return ret;
	}
	LOG_DBG("VAL IR, %u", measurements[0].val1);

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_BLUE, &measurements[1]);
	if (ret) {
		LOG_ERR("Error %d: get blue channel failed", ret);
		return ret;
	}
	LOG_DBG("VAL B, %d", measurements[1].val1);

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_GREEN, &measurements[2]);
	if (ret) {
		LOG_ERR("Error %d: get green channel failed", ret);
		return ret;
	}
	LOG_DBG("VAL G, %d", measurements[2].val1);

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_RED, &measurements[3]);
	if (ret) {
		LOG_ERR("Error %d: get red channel failed", ret);
		return ret;
	}
	LOG_DBG("VAL R, %d", measurements[3].val1);

	return 0;
}

#ifdef CONFIG_LIGHT_SENSOR_TRIGGER_ENABLE
static void trigger_handler(const struct device *dev, struct sensor_trigger *trigger) 
{
	ARG_UNUSED(dev);
	switch (trigger->type)
	{
	case SENSOR_TRIG_THRESHOLD:
	{
		int ret; 

		LOG_DBG("Threshold trigger fired!");

		// TODO: Generate sensor event

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

		// TODO: Generate sensor event

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
#endif /* CONFIG_LIGHT_SENSOR_TRIGGER_ENABLE */

int light_sensor_read(struct sensor_value light_values[], size_t size)
{
	int ret;

	if ((size / sizeof(struct sensor_value)) != 4) {
		LOG_ERR("Error %d: Invalid array size", -EINVAL);
		return -EINVAL;
	}

	ret = sensor_read(light_values);
	if (ret == -EBUSY) {
		return ret;
	}
	else if (ret) {
		LOG_ERR("Error %d: sensor read failed", ret);
		return ret;
	}

	return 0;
}

int colour_sensor_read(struct sensor_value colour_values[], size_t size)
{
	int ret;

	if ((size / sizeof(struct sensor_value)) != 4) {
		LOG_ERR("Error %d: Invalid array size", -EINVAL);
		return -EINVAL;
	}

	ui_sense_led_on_off(true);
	k_sleep(K_MSEC(SENSE_LED_ON_TIME_MS));

	ret = sensor_read(colour_values);

	ui_sense_led_on_off(false);

	if (ret == -EBUSY) {
		return ret;
	}
	else if (ret) {
		LOG_ERR("Error %d: sensor read failed", ret);
		return ret;
	}
	
	return 0;
}

int light_sensor_init(void)
{
	fetch_timestamp = 0;

#ifdef CONFIG_COLOUR_SENSOR	
	ui_sense_led_init();
#endif

	light_sensor_dev = device_get_binding(LIGHT_SENSOR_NAME);
	if (!light_sensor_dev) {
		LOG_ERR("Error %d: could not bind to Light Sensor device", -ENODEV);
		return -ENODEV;
	}

#ifdef CONFIG_LIGHT_SENSOR_TRIGGER_ENABLE
	int ret;
#ifdef CONFIG_LIGHT_SENSOR_TRIGGER_THRESH
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
#endif /* CONFIG_LIGHT_SENSOR_TRIGGER_THRESH */
	
	ret = sensor_trigger_set(light_sensor_dev, &sensor_trigger_config,
					trigger_handler);
	if (ret) {
		LOG_ERR("Error %d: set trigger handler failed", ret);
		return ret;
	}
#endif /* CONFIG_LIGHT_SENSOR_TRIGGER_ENABLE */

	return 0;
}