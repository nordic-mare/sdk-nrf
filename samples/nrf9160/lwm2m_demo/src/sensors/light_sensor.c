/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "ui_sense_led.h"

#define MODULE light_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#if defined(CONFIG_LIGHT_SENSOR_USE_EXTERNAL)
#define LIGHT_SENSOR_NODE DT_PATH(soc, peripheral_40000000, i2c_a000, bh1749_38)
#define LIGHT_SENSOR_DEV_LABEL DT_LABEL(LIGHT_SENSOR_NODE)
#elif defined(CONFIG_LIGHT_SENSOR_USE_SIM)
#define LIGHT_SIM_BASE CONFIG_LIGHT_SENSOR_LIGHT_SIM_VAL
#define COLOUR_SIM_BASE CONFIG_LIGHT_SENSOR_COLOUR_SIM_VAL
#define LIGHT_SIM_MAX_DIFF CONFIG_LIGHT_SENSOR_LIGHT_SIM_MAX_DIFF
#define COLOUR_SIM_MAX_DIFF CONFIG_LIGHT_SENSOR_COLOUR_SIM_MAX_DIFF
#endif

/* Macros to scale light and colour measurements to uint8_t */
#define SCALE_LIGHT_MEAS(raw_value)                                                                \
	MIN((raw_value * 255U / CONFIG_LIGHT_SENSOR_LIGHT_MEASUREMENT_MAX_VALUE), UINT8_MAX)
#define SCALE_COLOUR_MEAS(raw_value)                                                               \
	MIN((raw_value * 255U / CONFIG_LIGHT_SENSOR_COLOUR_MEASUREMENT_MAX_VALUE), UINT8_MAX)

#define NUM_CHANNELS 4U

#define RGBIR_STR_LENGTH 11U /* Format: '0xRRGGBBIR\0'. */

#define SENSOR_FETCH_DELAY_MS 200U /* Time before a new fetch can be tried. */
#define SENSE_LED_ON_TIME_MS 300U /* Time the sense LED stays on during colour read. */

#if defined(CONFIG_LIGHT_SENSOR_USE_EXTERNAL)
static const struct device *light_sensor_dev;
static int64_t fetch_timestamp;

/* sensor_sample_fetch_chan fails if called multiple times in a row with no delay. */
static bool fetch_ready(void)
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
	} else {
		LOG_DBG("Sensor fetch not ready");
		return -EBUSY;
	}

	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_IR, &measurements[0]);
	if (ret) {
		LOG_ERR("Error %d: get IR channel failed", ret);
		return ret;
	}
	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_BLUE, &measurements[1]);
	if (ret) {
		LOG_ERR("Error %d: get blue channel failed", ret);
		return ret;
	}
	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_GREEN, &measurements[2]);
	if (ret) {
		LOG_ERR("Error %d: get green channel failed", ret);
		return ret;
	}
	ret = sensor_channel_get(light_sensor_dev, SENSOR_CHAN_RED, &measurements[3]);
	if (ret) {
		LOG_ERR("Error %d: get red channel failed", ret);
		return ret;
	}

	return 0;
}
#endif /* if defined(CONFIG_LIGHT_SENSOR_USE_EXTERNAL) */

int light_sensor_read(uint32_t *light_value)
{
	struct sensor_value light_values[NUM_CHANNELS];
	uint32_t scaled_measurement;

#if defined(CONFIG_LIGHT_SENSOR_USE_EXTERNAL)
	int ret;

	ret = sensor_read(light_values);
	if (ret == -EBUSY) {
		return ret;
	} else if (ret) {
		LOG_ERR("Error %d: sensor read failed", ret);
		return ret;
	}

#elif defined(CONFIG_LIGHT_SENSOR_USE_SIM)
	/* TODO: Simulate with rng */
	for (int i = 0; i < NUM_CHANNELS; i++) {
		light_values[i].val1 = MAX(0, LIGHT_SIM_BASE + (rand() % LIGHT_SIM_MAX_DIFF) *
								       (1 - 2 * (rand() % 2)));
	}
#endif

	*light_value = 0;

	/* Scale measurements and combine in 4-byte light value, one byte per colour channel */
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		scaled_measurement = SCALE_LIGHT_MEAS(light_values[i].val1);
		*light_value |= scaled_measurement << 8 * i;
	}

	LOG_INF("Light sensor: read 0x%08X", *light_value);

	return 0;
}

int colour_sensor_read(uint32_t *colour_value)
{
	struct sensor_value colour_values[NUM_CHANNELS];
	uint32_t scaled_measurement;

#if defined(CONFIG_LIGHT_SENSOR_USE_EXTERNAL)
	int ret;

	ui_sense_led_on_off(true);
	k_sleep(K_MSEC(SENSE_LED_ON_TIME_MS));

	ret = sensor_read(colour_values);

	ui_sense_led_on_off(false);

	if (ret == -EBUSY) {
		return ret;
	} else if (ret) {
		LOG_ERR("Error %d: sensor read failed", ret);
		return ret;
	}
#elif defined(CONFIG_LIGHT_SENSOR_USE_SIM)
	/* TODO: Simulate with rng */
	for (int i = 0; i < NUM_CHANNELS; i++) {
		colour_values[i].val1 = MAX(0, COLOUR_SIM_BASE + (rand() % COLOUR_SIM_MAX_DIFF) *
									 (1 - 2 * (rand() % 2)));
	}
#endif

	*colour_value = 0;

	/* Scale measurements and combine in 4-byte colour value, one byte per colour channel */
	for (int i = 0; i < 4; ++i) {
		scaled_measurement = SCALE_COLOUR_MEAS(colour_values[i].val1);
		*colour_value |= scaled_measurement << 8 * i;
	}

	LOG_INF("Colour sensor: read 0x%08X", *colour_value);

	return 0;
}

int light_sensor_init(void)
{
#if defined(CONFIG_LIGHT_SENSOR_USE_EXTERNAL)
	fetch_timestamp = 0;

	ui_sense_led_init();

	light_sensor_dev = device_get_binding(LIGHT_SENSOR_DEV_LABEL);
	if (!light_sensor_dev) {
		LOG_ERR("Error %d: could not bind to Light Sensor device", -ENODEV);
		return -ENODEV;
	}
#endif

	return 0;
}
