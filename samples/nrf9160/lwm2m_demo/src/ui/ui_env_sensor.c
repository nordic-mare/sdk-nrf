#include <zephyr.h>
#include <drivers/sensor.h>
#include <net/lwm2m.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_env_sens, CONFIG_APP_LOG_LEVEL);

#define ENV_SENSOR_NODE	DT_PATH(soc, peripheral_40000000, i2c_a000, bme680_76)
#define ENV_SENSOR_NAME	DT_LABEL(ENV_SENSOR_NODE)

static const struct device *env_sensor_dev;
static struct sensor_value sensor_val;


int ui_env_sensor_init(void)
{
    env_sensor_dev = device_get_binding(ENV_SENSOR_NAME);
	if (!env_sensor_dev) {
		LOG_ERR("No enviroment sensor device found.");
        return -ENODEV;
	}

    return 0;
}



int ui_read_env_sensor(char *unit, enum sensor_channel channel, float32_value_t *float_val)
{
	const char *name = env_sensor_dev->name;
	struct sensor_value sensor_val;
	int ret;

	ret = sensor_sample_fetch(env_sensor_dev);
	if (ret) {
		LOG_ERR("%s: I/O error: %d", name, ret);
		return ret;
	}

	ret = sensor_channel_get(env_sensor_dev, channel, &sensor_val);
	if (ret) {
		LOG_ERR("%s: can't get data: %d", name, ret);
		return ret;
	}

	LOG_DBG("%s: read %d.%d %s", name, sensor_val.val1, sensor_val.val2, unit);
	float_val->val1 = sensor_val.val1;
	float_val->val2 = sensor_val.val2;

	return 0;
}