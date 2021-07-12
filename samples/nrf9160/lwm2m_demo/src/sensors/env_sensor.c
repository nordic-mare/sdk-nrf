#include <zephyr.h>
#include <drivers/sensor.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(env_sensor, CONFIG_APP_LOG_LEVEL);

#define ENV_SENSOR_NODE	DT_PATH(soc, peripheral_40000000, i2c_a000, bme680_76)
#define ENV_SENSOR_NAME	DT_LABEL(ENV_SENSOR_NODE)

#define TEMP_UNIT 		"C"
#define PRESS_UNIT 		"kPa"
#define HUMID_UNIT		"%"
#define GAS_RES_UNIT 	"Ohm"

static const struct device *env_sensor_dev;
static struct sensor_value env_sensor_val;

static int read_sensor(enum sensor_channel channel)
{
	int ret;

	ret = sensor_sample_fetch(env_sensor_dev);
	if (ret) {
		LOG_ERR("Error %d: fetch sample failed", ret);
		return ret;
	}

	ret = sensor_channel_get(env_sensor_dev, channel, &env_sensor_val);
	if (ret) {
		LOG_ERR("Error %d: get channel failed", ret);
		return ret;
	}

	return 0;
}

int env_sensor_read_temp(int32_t *temp_float_val1, int32_t *temp_float_val2)
{
	int ret;

	ret = read_sensor(SENSOR_CHAN_AMBIENT_TEMP);
	if (ret) {
		LOG_ERR("Error %d: read temperatur sensor failed", ret);
		return ret;
	}

	LOG_DBG("%s: read %d.%d %s", env_sensor_dev->name, 
			env_sensor_val.val1, env_sensor_val.val2, TEMP_UNIT);

	*temp_float_val1 = env_sensor_val.val1;
	*temp_float_val2 = env_sensor_val.val2;

	return 0;
}

int env_sensor_read_pressure(int32_t *press_float_val1, int32_t *press_float_val2)
{
	int ret;

	ret = read_sensor(SENSOR_CHAN_PRESS);
	if (ret) {
		LOG_ERR("Error %d: read pressure sensor failed", ret);
		return ret;
	}

	LOG_DBG("%s: read %d.%d %s", env_sensor_dev->name, 
			env_sensor_val.val1, env_sensor_val.val2, PRESS_UNIT);

	*press_float_val1 = env_sensor_val.val1;
	*press_float_val2 = env_sensor_val.val2;

	return 0;
}

int env_sensor_read_humidity(int32_t *humid_float_val1, int32_t *humid_float_val2)
{
	int ret;

	ret = read_sensor(SENSOR_CHAN_HUMIDITY);
	if (ret) {
		LOG_ERR("Error %d: read humidity sensor failed", ret);
		return ret;
	}

	LOG_DBG("%s: read %d.%d %s", env_sensor_dev->name, 
			env_sensor_val.val1, env_sensor_val.val2, HUMID_UNIT);

	*humid_float_val1 = env_sensor_val.val1;
	*humid_float_val2 = env_sensor_val.val2;

	return 0;
}

int env_sensor_read_gas_resistance(int32_t *gas_res_float_val1, int32_t *gas_res_float_val2)
{
	int ret;

	ret = read_sensor(SENSOR_CHAN_GAS_RES);
	if (ret) {
		LOG_ERR("Error %d: read gas resistance sensor failed", ret);
		return ret;
	}

	LOG_DBG("%s: read %d.%d %s", env_sensor_dev->name, 
			env_sensor_val.val1, env_sensor_val.val2, GAS_RES_UNIT);

	*gas_res_float_val1 = env_sensor_val.val1;
	*gas_res_float_val2 = env_sensor_val.val2;

	return 0;
}

int env_sensor_init(void)
{
	env_sensor_dev = device_get_binding(ENV_SENSOR_NAME);
	if (!env_sensor_dev) {
		LOG_ERR("Error %d: could not bind to Environment Sensor device", -ENODEV);
		return -ENODEV;
	}

	return 0;
}