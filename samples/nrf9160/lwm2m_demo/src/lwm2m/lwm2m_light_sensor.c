/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <net/lwm2m.h>
#include <lwm2m_resource_ids.h>
#include <stdio.h>

#include "measurement_event.h"
#include "light_sensor.h"

#define MODULE app_lwm2m_light_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define LWM2M_RES_DATA_FLAG_RW	0

#define IPSO_COLOUR_OBJECT_ID 3335

#define LIGHT_OBJ_INSTANCE    0
#define COLOUR_OBJ_INSTANCE   1

#define RGBIR_STR_LENGTH      11  // '0xRRGGBBIR\0'

#define LIGHT_SENSOR_APP_NAME   "Light sensor"
#define COLOUR_SENSOR_APP_NAME  "Colour sensor"
#define LIGHT_UNIT              "RGB-IR"

static char light_value_str[RGBIR_STR_LENGTH] = "Unread";
static char colour_value_str[RGBIR_STR_LENGTH] = "Unread";


static void *light_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len) 
{
    int ret;
    uint32_t light_measurement;

    ret = light_sensor_read(&light_measurement);
    if (ret) {
        LOG_ERR("Error %d: read light sensor failed", ret);
        return NULL;
    }

    snprintf(light_value_str, RGBIR_STR_LENGTH,    
                "0x%08X", light_measurement);

    return &light_value_str;
}


static void *colour_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
    int ret;
    uint32_t colour_measurement;

    ret = colour_sensor_read(&colour_measurement);
    if (ret) {
        LOG_ERR("Error %d: read colour sensor failed", ret);
        return NULL;
    }

    snprintf(colour_value_str, RGBIR_STR_LENGTH,
                "0x%08X", colour_measurement);

    return &colour_value_str;
}


int lwm2m_init_light_sensor(void) 
{
    light_sensor_init();    

    /* Ambient light sensor */
    lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE));
    lwm2m_engine_register_read_callback(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, COLOUR_RID), 
            light_sensor_read_cb);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, COLOUR_RID),
            &light_value_str, RGBIR_STR_LENGTH, LWM2M_RES_DATA_FLAG_RW);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, APPLICATION_TYPE_RID),
            LIGHT_SENSOR_APP_NAME, 
            sizeof(LIGHT_SENSOR_APP_NAME),
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, SENSOR_UNITS_RID), 
            LIGHT_UNIT, 
            sizeof(LIGHT_UNIT),
            LWM2M_RES_DATA_FLAG_RO);

    /* Surface colour sensor */
    lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INSTANCE));
    lwm2m_engine_register_read_callback(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INSTANCE, COLOUR_RID), 
            colour_sensor_read_cb);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INSTANCE, COLOUR_RID),
            &colour_value_str, RGBIR_STR_LENGTH, LWM2M_RES_DATA_FLAG_RW);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INSTANCE, APPLICATION_TYPE_RID),
            COLOUR_SENSOR_APP_NAME, 
            sizeof(COLOUR_SENSOR_APP_NAME),
            LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INSTANCE, SENSOR_UNITS_RID), 
            LIGHT_UNIT, 
            sizeof(LIGHT_UNIT),
            LWM2M_RES_DATA_FLAG_RO);
    
    return 0;
}


static bool event_handler(const struct event_header *eh)
{
    if (is_measurement_event(eh)) {
        struct measurement_event *event = cast_measurement_event(eh);
        char measurement_value_str[RGBIR_STR_LENGTH];

        snprintf(measurement_value_str, RGBIR_STR_LENGTH,
                    "0x%08X", event->unsigned_val);

        switch (event->type)
        {
        case LightMeasurement:
            LOG_DBG("Light measurement event received! Val: %s", measurement_value_str);

            lwm2m_engine_set_string(
                LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, COLOUR_RID),
                measurement_value_str);
            break;
        
        case ColourMeasurement:
            LOG_DBG("Colour measurement event received! Val: %s", measurement_value_str);

            lwm2m_engine_set_string(
                LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, COLOUR_OBJ_INSTANCE, COLOUR_RID),
                measurement_value_str);
            break;

        default:
            return false;
        }

        return true;
    }

    return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, measurement_event);