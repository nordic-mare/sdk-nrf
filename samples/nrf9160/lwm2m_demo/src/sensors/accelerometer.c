#include <zephyr.h>
#include <drivers/sensor.h>

#include "accelerometer.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(accelerometer, CONFIG_APP_LOG_LEVEL);

#if defined(CONFIG_BOARD_THINGY91_NRF9160NS)
#define ACCEL_NODE          DT_PATH(soc, peripheral_40000000, spi_b000, adxl362_0)
#define ACCEL_DEV_LABEL     DT_LABEL(ACCEL_NODE)
#elif defined(CONFIG_BOARD_NRF9160DK_NRF9160NS)
#define ACCEL_DEV_LABEL     CONFIG_SENSOR_SIM_DEV_NAME
#endif

#define CALIBRATION_ITERATIONS      CONFIG_ACCEL_CALIBRATION_ITERATIONS

static const struct device *accel_dev;

static double accel_offset[3];

int accelerometer_read(struct accelerometer_sensor_data *data)
{
    int ret;
    double x_temp, y_temp, z_temp;

    ret = sensor_sample_fetch(accel_dev);
    if (ret) {
        LOG_ERR("Error %d: fetch sample failed", ret);
        return ret;
    }

    ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_X,
                            &(data->x));
    if(ret) {
        LOG_ERR("Error %d: get x channel failed", ret);
        return ret;
    }
    LOG_DBG("x: val1=%d, val2=%d", data->x.val1, data->x.val2);
    ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_Y,
                            &(data->y));
    if(ret) {
        LOG_ERR("Error %d: get y channel failed", ret);
        return ret;
    }
    LOG_DBG("y: val1=%d, val2=%d", data->y.val1, data->y.val2);
    ret = sensor_channel_get(accel_dev, SENSOR_CHAN_ACCEL_Z,
                            &(data->z));
    if(ret) {
        LOG_ERR("Error %d: get z channel failed", ret);
        return ret;
    }
    LOG_DBG("z: val1=%d, val2=%d", data->z.val1, data->z.val2);

    /* Adjust for sensor bias */
    x_temp = sensor_value_to_double(&(data->x)) - accel_offset[0];
	y_temp = sensor_value_to_double(&(data->y)) - accel_offset[1];
	z_temp = sensor_value_to_double(&(data->z)) - accel_offset[2];
    sensor_value_from_double(&(data->x), x_temp);
    sensor_value_from_double(&(data->y), y_temp);
    sensor_value_from_double(&(data->z), z_temp);

    return 0;
}

int accelerometer_calibrate(void)
{
    int ret;
    struct accelerometer_sensor_data accel_data;
    double aggr_data[3] = {0};

    /* What does this do? */
    k_sleep(K_SECONDS(2));

    for (int i = 0; i < CALIBRATION_ITERATIONS; i++) {
        ret = accelerometer_read(&accel_data);
        if (ret) {
            LOG_ERR("Error %d: read accelerometer failed", ret);
            return ret;
        }

        aggr_data[0] += sensor_value_to_double(&(accel_data.x));
        aggr_data[1] += sensor_value_to_double(&(accel_data.y));
        aggr_data[2] += sensor_value_to_double(&(accel_data.z))
                        + ((double)SENSOR_G) / 1000000.0;
    }

    accel_offset[0] = aggr_data[0] / (double)CALIBRATION_ITERATIONS;
	accel_offset[1] = aggr_data[1] / (double)CALIBRATION_ITERATIONS;
	accel_offset[2] = aggr_data[2] / (double)CALIBRATION_ITERATIONS;

    return 0;
}

int accelerometer_init(void)
{
    int ret;

	accel_dev = device_get_binding(ACCEL_DEV_LABEL);
	if (!accel_dev) {
		LOG_ERR("Error %d: could not get accelerometer device", -ENODEV);
		return -ENODEV;
	}

	if (IS_ENABLED(CONFIG_ACCEL_CALIBRATE)) {
		ret = accelerometer_calibrate();
		if (ret) {
			LOG_ERR("Error %d: calibrate accelerometer failed", ret);
			return ret;
		}
	}

    return 0;
}