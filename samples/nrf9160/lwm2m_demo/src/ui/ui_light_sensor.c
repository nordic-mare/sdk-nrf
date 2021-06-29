#include <zephyr.h>
#include <drivers/sensor.h>

#define LIGHT_SENSOR_NODE_ID	DT_PATH(soc, peripheral_40000000, i2c_a000, bh1749_38)
#define LIGHT_SENSOR_NAME	    DT_LABEL(LIGHT_SENSOR_NODE_ID)

#define RGBIR_STR_LENGTH        11  // '0xRRGGBBIR\0'

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_light_sensor, CONFIG_APP_LOG_LEVEL);

static const struct device *light_sens_dev;
struct sensor_value sensor_val;


int ui_light_sensor_init(void)
{
    light_sens_dev = device_get_binding(LIGHT_SENSOR_NAME);
    if (!light_sens_dev) {
        LOG_ERR("No light sensor found");
        return -ENODEV;
    }

    return 0;
}


int ui_light_sensor_read(char *light_value)
{
    int ret;
    uint8_t red_val, blue_val, green_val, ir_val;

    ret = sensor_sample_fetch_chan(light_sens_dev, SENSOR_CHAN_ALL);
    if (ret) {
        LOG_ERR("Could not fetch sample");
        return ret;
    }
    
    ret = sensor_channel_get(light_sens_dev, SENSOR_CHAN_RED, &sensor_val);
    if (ret) {
        LOG_ERR("Could not get red channel");
        return ret;
    }
    red_val = sensor_val.val1;
    LOG_DBG("Light sensor red value: %i", sensor_val.val1);

    ret = sensor_channel_get(light_sens_dev, SENSOR_CHAN_GREEN, &sensor_val);
    if (ret) {
        LOG_ERR("Could not get green channel");
        return ret;
    }
    green_val = sensor_val.val1;
    LOG_DBG("Light sensor green value: %i", green_val);

    ret = sensor_channel_get(light_sens_dev, SENSOR_CHAN_BLUE, &sensor_val);
    if (ret) {
        LOG_ERR("Could not get blue channel");
        return ret;
    }
    blue_val = sensor_val.val1;
    LOG_DBG("Light sensor blue value: %i", blue_val);

    ret = sensor_channel_get(light_sens_dev, SENSOR_CHAN_IR, &sensor_val);
    if (ret) {
        LOG_ERR("Could not get infra red channel");
        return ret;
    }
    ir_val = sensor_val.val1;
    LOG_DBG("Light sensor IR value: %i", ir_val);

    snprintf(light_value, RGBIR_STR_LENGTH, 
            "0x%02X%02X%02X%02X", red_val, green_val, blue_val, ir_val);

    return 0;
}