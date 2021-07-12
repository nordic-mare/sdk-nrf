#include <zephyr.h>
#include <drivers/sensor.h>
#include <math.h>

#include "sensor_event.h"
#include "light_sensor.h"
#include "env_sensor.h"

#define MODULE sensor_module

#include <logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_APP_LOG_LEVEL);

#define TEMP_DELAY          CONFIG_SENSOR_MODULE_TEMP_DELAY
#define TEMP_DELTA_VAL1     (CONFIG_SENSOR_MODULE_TEMP_DELTA / 100)
#define TEMP_DELTA_VAL2     ((CONFIG_SENSOR_MODULE_TEMP_DELTA % 100) * 10000) // DOUBLECHECK THIS

static struct k_work_delayable light_work;
static struct k_work_delayable colour_work;
static struct k_work_delayable temp_work;
static struct k_work_delayable press_work;
static struct k_work_delayable humid_work;
static struct k_work_delayable gas_res_work;

static void light_work_cb(struct k_work *work);

static void colour_work_cb(struct k_work *work);

static bool float32_sufficient_diff(uint32_t diff_val1, uint32_t diff_val2, 
						uint32_t req_diff_va1, uint32_t req_diff_val2)
{
	if (diff_val1 > req_diff_va1) {
		return true;
	}
	else if (diff_val1 == req_diff_va1 && diff_val2 >= req_diff_val2) {
		return true;
	}
	else {
		return false;
	}
}

static void temp_work_cb(struct k_work *work) 
{
	struct sensor_value temp_val;
	int32_t old_temp_val1 = 0; 
	int32_t old_temp_val2 = 0;

	env_sensor_read_temp(&temp_val);

	LOG_DBG("Temperature work: %d.%d", temp_val.val1, temp_val.val2);

	uint32_t diff1 = fabs(temp_val.val1 - old_temp_val1);
	uint32_t diff2 = fabs(temp_val.val2 - old_temp_val2);

	if (float32_sufficient_diff(diff1, diff2, TEMP_DELTA_VAL1, TEMP_DELTA_VAL2)) {
		LOG_DBG("CREATE TEMP EVENT");
	}

	k_work_schedule(&temp_work, K_SECONDS(10));
}

static void press_work_cb(struct k_work *work);

static void humid_work_cb(struct k_work *work);

static void gas_res_work_cb(struct k_work *work);

int sensor_module_init(void)
{
	k_work_init_delayable(&temp_work, temp_work_cb);

	//k_work_schedule(&temp_work, K_NO_WAIT);

	return 0;
} 