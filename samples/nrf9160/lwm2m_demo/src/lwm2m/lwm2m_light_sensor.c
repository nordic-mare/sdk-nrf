/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <net/lwm2m.h>
#include <stdio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_light_sens, CONFIG_APP_LOG_LEVEL);

#define LIGHT_SENSOR_NODE	DT_PATH(soc, peripheral_40000000, i2c_a000, bh1749_38)
#define LIGHT_SENSOR_NAME	DT_LABEL(LIGHT_SENSOR_NODE)

#define APP_NAME "Light Sensor"

static const struct device *light_sens_dev;
static char colour_value[11];

static void* light_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len) {
    uint8_t red_val, blue_val, green_val, ir_val;
    struct sensor_value sens_val;

	/* Only object instance 0 is currently used */
	if (obj_inst_id != 0) {
		*data_len = 0;
		return NULL;
	}
    sensor_sample_fetch_chan(light_sens_dev, SENSOR_CHAN_ALL);
    sensor_channel_get(light_sens_dev, SENSOR_CHAN_RED, &sens_val);
    red_val = sens_val.val1;
    LOG_DBG("Light sensor red value: %i", red_val);
    sensor_channel_get(light_sens_dev, SENSOR_CHAN_GREEN, &sens_val);
    green_val = sens_val.val1;
    LOG_DBG("Light sensor green value: %i", green_val);
    sensor_channel_get(light_sens_dev, SENSOR_CHAN_BLUE, &sens_val);
    blue_val = sens_val.val1;
    LOG_DBG("Light sensor blue value: %i", blue_val);
    sensor_channel_get(light_sens_dev, SENSOR_CHAN_IR, &sens_val);
    ir_val = sens_val.val1;
    LOG_DBG("Light sensor IR value: %i", ir_val);
    snprintf(colour_value, 11, "0x%02X%02X%02X%02X", red_val, green_val, blue_val, ir_val);
    return colour_value;
}

int lwm2m_init_light_sensor(void) {
    light_sens_dev = device_get_binding(LIGHT_SENSOR_NAME);
    if (!light_sens_dev) {
        LOG_ERR("No light sensor found");
    }
    char *light_unit = "RGB-IR";
    lwm2m_engine_create_obj_inst("3335/0");
    lwm2m_engine_register_read_callback("3335/0/5706", light_read_cb);
    lwm2m_engine_set_res_data("3335/0/5750",
                APP_NAME, sizeof(APP_NAME),
                LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data("3335/0/5701", light_unit, sizeof(light_unit),
                LWM2M_RES_DATA_FLAG_RO);
    
    return 0;
}