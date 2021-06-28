/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <net/lwm2m.h>
#include <stdio.h>

#include "lwm2m_resource_ids.h"
#include "lwm2m_additional_ids.h"

#include <ui_sense_led.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_light_sens, CONFIG_APP_LOG_LEVEL);

#define LIGHT_SENSOR_NODE_ID	DT_PATH(soc, peripheral_40000000, i2c_a000, bh1749_38)
#define LIGHT_SENSOR_NAME	    DT_LABEL(LIGHT_SENSOR_NODE_ID)

#define LIGHT_OBJ_INST    0
#define COLOUR_OBJ_INST   1

#define LIGHT_VALUE_STR_LENGTH      11  // '0xRRGGBBIR\0'

#define LIGHT_SENSOR_APP_NAME   "Light sensor"
#define COLOUR_SENSOR_APP_NAME  "Colour sensor"
#define LIGHT_UNIT              "RGB-IR"

#define MEAS_QUAL_IND_UNCHECKED     0
#define MEAS_QUAL_LEVEL_UNCHECKED   0

#define SENSE_LED_ON_TIME_MS        500

static const struct device *light_sens_dev;
static char light_value[LIGHT_VALUE_STR_LENGTH];


static void *light_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len) 
{
    int32_t ts;
    uint8_t red_val, blue_val, green_val, ir_val;
    struct sensor_value sens_val;

    if (obj_inst_id == COLOUR_OBJ_INST) {
        ui_sense_led_on_off(true);
        k_sleep(K_MSEC(SENSE_LED_ON_TIME_MS));
    }

    sensor_sample_fetch_chan(light_sens_dev, SENSOR_CHAN_ALL);

    if (obj_inst_id == COLOUR_OBJ_INST) {
        ui_sense_led_on_off(false);
    }

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

    snprintf(light_value, LIGHT_VALUE_STR_LENGTH, "0x%02X%02X%02X%02X", red_val, green_val, blue_val, ir_val);

    /* get current time from device */
	lwm2m_engine_get_s32("3/0/13", &ts);
	/* set timestamp */
    char obj_inst_id_str[16];
    snprintf(
            obj_inst_id_str, 
            16, 
            "3335/%d/5518", 
            obj_inst_id);
	lwm2m_engine_set_s32(obj_inst_id_str, ts);

    return &light_value;
}


int lwm2m_init_light_sensor(void) {
    light_sens_dev = device_get_binding(LIGHT_SENSOR_NAME);
    if (!light_sens_dev) {
        LOG_ERR("No light sensor found");
    }

    ui_sense_led_init();

    /* Ambient light sensor */
    lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INST));
    lwm2m_engine_register_read_callback(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INST, COLOUR_RID), 
            light_sensor_read_cb);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INST, APPLICATION_TYPE_RID),
            LIGHT_SENSOR_APP_NAME, 
            sizeof(LIGHT_SENSOR_APP_NAME),
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INST, SENSOR_UNITS_RID), 
            LIGHT_UNIT, 
            sizeof(LIGHT_UNIT),
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INST, MEASUREMENT_QUALITY_INDICATOR_RID), 
            MEAS_QUAL_IND_UNCHECKED, 
            sizeof(MEAS_QUAL_IND_UNCHECKED), 
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INST, MEASUREMENT_QUALITY_LEVEL_RID), 
            MEAS_QUAL_LEVEL_UNCHECKED, 
            sizeof(MEAS_QUAL_LEVEL_UNCHECKED), 
            LWM2M_RES_DATA_FLAG_RO);

    /* Surface colour sensor */
    lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INST));
    lwm2m_engine_register_read_callback(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INST, COLOUR_RID), 
            light_sensor_read_cb);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INST, APPLICATION_TYPE_RID),
            COLOUR_SENSOR_APP_NAME, 
            sizeof(COLOUR_SENSOR_APP_NAME),
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INST, SENSOR_UNITS_RID), 
            LIGHT_UNIT, 
            sizeof(LIGHT_UNIT),
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INST, MEASUREMENT_QUALITY_INDICATOR_RID), 
            MEAS_QUAL_IND_UNCHECKED, 
            sizeof(MEAS_QUAL_IND_UNCHECKED), 
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INST, MEASUREMENT_QUALITY_LEVEL_RID), 
            MEAS_QUAL_LEVEL_UNCHECKED, 
            sizeof(MEAS_QUAL_LEVEL_UNCHECKED), 
            LWM2M_RES_DATA_FLAG_RO);
    
    return 0;
}