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
#include "ui_light_sensor.h"

#define MODULE app_lwm2m_light_sensor

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define IPSO_COLOUR_OBJECT_ID 3335

#define LIGHT_OBJ_INSTANCE    0
#define COLOUR_OBJ_INSTANCE   1

#define RGBIR_STR_LENGTH      11  // '0xRRGGBBIR\0'

#define LIGHT_SENSOR_APP_NAME   "Light sensor"
#define COLOUR_SENSOR_APP_NAME  "Colour sensor"
#define LIGHT_UNIT              "RGB-IR"

static char light_value[RGBIR_STR_LENGTH] = "Unread";
static char colour_value[RGBIR_STR_LENGTH] = "Unread";


static void *light_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len) 
{
    uint32_t measurement;

    ui_light_sensor_read(&measurement);

    snprintf(light_value, RGBIR_STR_LENGTH,
                    "0x%08X", measurement);

    return &light_value;
}


static void *colour_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len)
{
    uint32_t measurement;

    ui_colour_sensor_read(&measurement);

    snprintf(colour_value, RGBIR_STR_LENGTH,
                    "0x%08X", measurement);

    return &colour_value;
}


int lwm2m_init_light_sensor(void) 
{
    ui_light_sensor_init();    

    /* Ambient light sensor */
    lwm2m_engine_create_obj_inst(LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE));
    lwm2m_engine_register_read_callback(
            LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, COLOUR_RID), 
            light_sensor_read_cb);
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

        if (event->type == LightMeasurement) {
            LOG_DBG("Light measurement event received! Val: 0x%08X", event->unsigned_val);
            snprintf(light_value, RGBIR_STR_LENGTH,
                    "0x%08X", event->unsigned_val);
            lwm2m_engine_set_string(
                LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, COLOUR_RID),
                light_value);

            return true;
        }
        else if (event->type == ColourMeasurement) {
            LOG_DBG("Colour measurement event received! Val: 0x%08X", event->unsigned_val);
            snprintf(colour_value, RGBIR_STR_LENGTH,
                    "0x%08X", event->unsigned_val);
            lwm2m_engine_set_string(
                LWM2M_PATH(IPSO_COLOUR_OBJECT_ID, LIGHT_OBJ_INSTANCE, COLOUR_RID),
                colour_value);

            return true;
        }

        return false;
    }

    return false;
}

EVENT_LISTENER(MODULE, event_handler);
EVENT_SUBSCRIBE(MODULE, measurement_event);