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

#include <logging/log.h>
LOG_MODULE_REGISTER(app_lwm2m_light_sens, CONFIG_APP_LOG_LEVEL);

#define LIGHT_SENSOR_NODE_ID	DT_PATH(soc, peripheral_40000000, i2c_a000, bh1749_38)
#define LIGHT_SENSOR_NAME	    DT_LABEL(LIGHT_SENSOR_NODE_ID)

#define COLOUR_SENSOR_OBJ_INST_ID   1

#define LIGHT_VALUE_STR_LENGTH      11  // '0xRRGGBBIR\0'

#define LIGHT_SENSOR_APP_NAME   "Light sensor"
#define COLOUR_SENSOR_APP_NAME  "Colour sensor"
#define LIGHT_UNIT              "RGB-IR"

// GPIO sense leds are used by colour sensor object instance.
#define GPIO_SENSE_RED_NODE_ID      DT_NODELABEL(sense_red_led)
#define GPIO_SENSE_GREEN_NODE_ID    DT_NODELABEL(sense_green_led)
#define GPIO_SENSE_BLUE_NODE_ID     DT_NODELABEL(sense_blue_led)

#define GPIO_NAME  DT_GPIO_LABEL(GPIO_SENSE_RED_NODE_ID, gpios)

#define RED_SENSE_LED_GPIO_PIN      DT_GPIO_PIN(GPIO_SENSE_RED_NODE_ID, gpios)
#define GREEN_SENSE_LED_GPIO_PIN    DT_GPIO_PIN(GPIO_SENSE_GREEN_NODE_ID, gpios)
#define BLUE_SENSE_LED_GPIO_PIN	    DT_GPIO_PIN(GPIO_SENSE_BLUE_NODE_ID, gpios)

#define SENSE_LED_GPIO_FLAGS	    DT_GPIO_FLAGS(GPIO_SENSE_RED_NODE_ID, gpios)


static const struct device *light_sens_dev;
static char light_value[LIGHT_VALUE_STR_LENGTH];

static const struct device *sense_led_dev;

static int gpio_sense_led_on_off(bool new_state)
{
    int ret;
    ret = gpio_pin_set(sense_led_dev, RED_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set red led");
        return ret;
    }
    ret = gpio_pin_set(sense_led_dev, GREEN_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set green led");
        return ret;
    }
    ret = gpio_pin_set(sense_led_dev, BLUE_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set blue led");
        return ret;
    }
    return 0;
}


int gpio_sense_led_init(void)
{
	int ret;

	sense_led_dev = device_get_binding(GPIO_NAME);
	if (!sense_led_dev) {
		return -ENODEV;
	}

	ret = gpio_pin_configure(sense_led_dev, RED_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
		return ret;
	}
    ret = gpio_pin_configure(sense_led_dev, GREEN_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
		return ret;
	}
    ret = gpio_pin_configure(sense_led_dev, BLUE_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		return ret;
	}

	return 0;
}


static void *light_sensor_read_cb(uint16_t obj_inst_id, uint16_t res_id, uint16_t res_inst_id,
			  size_t *data_len) 
{
    uint8_t red_val, blue_val, green_val, ir_val;
    struct sensor_value sens_val;

    if (obj_inst_id == COLOUR_SENSOR_OBJ_INST_ID) {
        gpio_sense_led_on_off(true);
        k_sleep(K_SECONDS(2));
    }

    sensor_sample_fetch_chan(light_sens_dev, SENSOR_CHAN_ALL);

    if (obj_inst_id == COLOUR_SENSOR_OBJ_INST_ID) {
        gpio_sense_led_on_off(false);
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

    return &light_value;
}


int lwm2m_init_light_sensor(void) {
    light_sens_dev = device_get_binding(LIGHT_SENSOR_NAME);
    if (!light_sens_dev) {
        LOG_ERR("No light sensor found");
    }

    gpio_sense_led_init();

    /* Ambient light sensor */
    lwm2m_engine_create_obj_inst("3335/0");
    lwm2m_engine_register_read_callback("3335/0/5706", light_sensor_read_cb);
    lwm2m_engine_set_res_data("3335/0/5750",
                LIGHT_SENSOR_APP_NAME, sizeof(LIGHT_SENSOR_APP_NAME),
                LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data("3335/0/5701", LIGHT_UNIT, sizeof(LIGHT_UNIT),
                LWM2M_RES_DATA_FLAG_RO);

    /* Surface colour sensor */
    lwm2m_engine_create_obj_inst("3335/1");
    lwm2m_engine_register_read_callback("3335/1/5706", light_sensor_read_cb);
    lwm2m_engine_set_res_data("3335/1/5750",
                COLOUR_SENSOR_APP_NAME, sizeof(COLOUR_SENSOR_APP_NAME),
                LWM2M_RES_DATA_FLAG_RO);
    lwm2m_engine_set_res_data("3335/1/5701", LIGHT_UNIT, sizeof(LIGHT_UNIT),
                LWM2M_RES_DATA_FLAG_RO);
    
    return 0;
}