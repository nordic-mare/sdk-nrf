/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <drivers/gps.h>
#include <drivers/sensor.h>
#include <console/console.h>
#include <net/cloud.h>
#include <dk_buttons_and_leds.h>
#include <modem/lte_lc.h>
#include <sys/reboot.h>
#include <modem/nrf_modem_lib.h>
#include <data/json.h>

#include "aggregator.h"
#include "ble.h"

#include "alarm.h"

/* Interval in milliseconds between each time status LEDs are updated. */
#define LEDS_UPDATE_INTERVAL K_MSEC(500)

/* Interval in microseconds between each time LEDs are updated when indicating
 * that an error has occurred.
 */
#define LEDS_ERROR_UPDATE_INTERVAL 250000

#define BUTTON_1 BIT(0)
#define BUTTON_2 BIT(1)
#define SWITCH_1 BIT(2)
#define SWITCH_2 BIT(3)

#define LED_ON(x) (x)
#define LED_BLINK(x) ((x) << 8)
#define LED_GET_ON(x) ((x)&0xFF)
#define LED_GET_BLINK(x) (((x) >> 8) & 0xFF)

/* Interval in milliseconds after the device will retry cloud connection
 * if the event NRF_CLOUD_EVT_TRANSPORT_CONNECTED is not received.
 */
#define RETRY_CONNECT_WAIT K_MSEC(90000)

enum {
	LEDS_INITIALIZING = LED_ON(0),
	LEDS_LTE_CONNECTING = LED_BLINK(DK_LED3_MSK),
	LEDS_LTE_CONNECTED = LED_ON(DK_LED3_MSK),
	LEDS_CLOUD_CONNECTING = LED_BLINK(DK_LED4_MSK),
	LEDS_CLOUD_PAIRING_WAIT = LED_BLINK(DK_LED3_MSK | DK_LED4_MSK),
	LEDS_CLOUD_CONNECTED = LED_ON(DK_LED4_MSK),
	LEDS_ERROR = LED_ON(DK_ALL_LEDS_MSK)
} display_state;

static struct cloud_backend *cloud_backend;

/* Sensor data */
static struct gps_nmea gps_nmea_data;
static struct cloud_sensor_data gps_cloud_data = {
	.type = "GPS",
	.tag = 0x1,
	.data = gps_nmea_data.buf,
	.length = GPS_NMEA_SENTENCE_MAX_LENGTH,
};
static atomic_val_t send_data_enable;

/* Structures for work */
static struct k_work_delayable leds_update_work;
static struct k_work_delayable connect_work;

static bool cloud_connected;

enum error_type {
	ERROR_NRF_CLOUD,
	ERROR_MODEM_RECOVERABLE,
};

struct cloud_data {
	const char *app_id;
	const char *data;
	const char *message_type;
};

struct json_obj_descr cloud_data_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct cloud_data, app_id, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct cloud_data, data, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct cloud_data, message_type, JSON_TOK_STRING)
};

/* Forward declaration of functions */
static void cloud_connect_work(struct k_work *work);
static void sensors_init(void);
static void work_init(void);

void cloud_update(struct cloud_sensor_data cloud_data);

/**@brief nRF Cloud error handler. */
void error_handler(enum error_type err_type, int err)
{
	if (err_type == ERROR_NRF_CLOUD) {
		int err;

		/* Turn off and shutdown modem */
		k_sched_lock();
		err = lte_lc_power_off();
		__ASSERT(err == 0, "lte_lc_power_off failed: %d", err);
		nrf_modem_lib_shutdown();
	}

#if !defined(CONFIG_DEBUG)
	sys_reboot(SYS_REBOOT_COLD);
#else
	uint8_t led_pattern;

	switch (err_type) {
	case ERROR_NRF_CLOUD:
		/* Blinking all LEDs ON/OFF in pairs (1 and 4, 2 and 3)
		 * if there is an application error.
		 */
		led_pattern = DK_LED1_MSK | DK_LED4_MSK;
		printk("Error of type ERROR_NRF_CLOUD: %d\n", err);
		break;
	case ERROR_MODEM_RECOVERABLE:
		/* Blinking all LEDs ON/OFF in pairs (1 and 3, 2 and 4)
		 * if there is a recoverable error.
		 */
		led_pattern = DK_LED1_MSK | DK_LED3_MSK;
		printk("Error of type ERROR_MODEM_RECOVERABLE: %d\n", err);
		break;
	default:
		/* Blinking all LEDs ON/OFF in pairs (1 and 2, 3 and 4)
		 * undefined error.
		 */
		led_pattern = DK_LED1_MSK | DK_LED2_MSK;
		break;
	}

	while (true) {
		dk_set_leds(led_pattern & 0x0f);
		k_busy_wait(LEDS_ERROR_UPDATE_INTERVAL);
		dk_set_leds(~led_pattern & 0x0f);
		k_busy_wait(LEDS_ERROR_UPDATE_INTERVAL);
	}
#endif /* CONFIG_DEBUG */
}

void nrf_cloud_error_handler(int err)
{
	error_handler(ERROR_NRF_CLOUD, err);
}

/**@brief Callback for GPS events */
static void gps_handler(const struct device *dev, struct gps_event *evt)
{
	uint32_t button_state, has_changed;
	struct sensor_data in_data = {
		.type = GPS_POSITION,
		.length = evt->nmea.len,
	};

	ARG_UNUSED(dev);

	switch (evt->type) {
	case GPS_EVT_SEARCH_STARTED:
		printk("GPS_EVT_SEARCH_STARTED\n");
		return;
	case GPS_EVT_SEARCH_STOPPED:
		printk("GPS_EVT_SEARCH_STOPPED\n");
		return;
	case GPS_EVT_SEARCH_TIMEOUT:
		printk("GPS_EVT_SEARCH_TIMEOUT\n");
		return;
	case GPS_EVT_PVT_FIX:
		printk("GPS_EVT_PVT_FIX\n");
		return;
	case GPS_EVT_NMEA_FIX:
		printk("GPS_EVT_NMEA_FIX\n");
		break;
	default:
		return;
	}

	dk_read_buttons(&button_state, &has_changed);

	if ((button_state & SWITCH_2) || !atomic_get(&send_data_enable)) {
		return;
	}

	gps_cloud_data.tag++;

	if (gps_cloud_data.tag == 0) {
		gps_cloud_data.tag = 0x1;
	}

	memcpy(&in_data.data[0], &gps_cloud_data.tag, sizeof(gps_cloud_data.tag));

	memcpy(&in_data.data[sizeof(gps_cloud_data.tag)], evt->nmea.buf, evt->nmea.len);

	if (aggregator_put(in_data) != 0) {
		printk("Failed to store GPS data.\n");
	}
}

/**@brief Update LEDs state. */
static void leds_update(struct k_work *work)
{
	static bool led_on;
	static uint8_t current_led_on_mask;
	uint8_t led_on_mask = current_led_on_mask;

	ARG_UNUSED(work);

	/* Reset LED3 and LED4. */
	led_on_mask &= ~(DK_LED3_MSK | DK_LED4_MSK);

	/* Set LED3 and LED4 to match current state. */
	led_on_mask |= LED_GET_ON(display_state);

	led_on = !led_on;
	if (led_on) {
		led_on_mask |= LED_GET_BLINK(display_state);
	} else {
		led_on_mask &= ~LED_GET_BLINK(display_state);
	}

	if (led_on_mask != current_led_on_mask) {
		dk_set_leds(led_on_mask);
		current_led_on_mask = led_on_mask;
	}

	k_work_schedule(&leds_update_work, LEDS_UPDATE_INTERVAL);
}

void cloud_update(struct cloud_sensor_data cloud_data) {
	int err;

	if (!cloud_connected) {
		printk("Not connected to cloud, aborting cloud publication\n");
		return;
	}

	struct cloud_data json_data = {
		.app_id = cloud_data.type,
		.data = cloud_data.data, // TODO: Fix
		.message_type = "DATA"
	};

	ssize_t json_len = json_calc_encoded_len(cloud_data_descr, ARRAY_SIZE(cloud_data_descr), &json_data) + 1;
	char json_buffer[json_len];

	err = json_obj_encode_buf(cloud_data_descr, ARRAY_SIZE(cloud_data_descr), &json_data, json_buffer, sizeof(json_buffer));
	if (err) {
		printk("Failed encoding sensor data as JSON: %d\n", err);
	}
	struct cloud_msg msg = {
		.qos = CLOUD_QOS_AT_MOST_ONCE,
		.buf = json_buffer,
		.len = json_len
	};

	if (strcmp(CONFIG_CLOUD_BACKEND, "NRF_CLOUD") == 0) {
		msg.endpoint.type = CLOUD_EP_MSG;
	} else {
		msg.endpoint.type = CLOUD_EP_STATE;
	}

	err = cloud_send(cloud_backend, &msg);
	if (err) {
		printk("Failed updating cloud, error %d", err);
	}
}

/**@brief Callback for Cloud events. */
void cloud_event_handler(const struct cloud_backend *const backend,
			 const struct cloud_event *const evt, void *user_data)
{
	switch (evt->type) {
	case CLOUD_EVT_CONNECTING:
		printk("CLOUD_EVT_CONNECTING\n");
		break;
	case CLOUD_EVT_CONNECTED:
		printk("CLOUD_EVT_CONNECTED\n");
		cloud_connected = true;
		/* This may fail if the work item is already being processed,
		 * but in such case, the next time the work handler is executed,
		 * it will exit after checking the above flag and the work will
		 * not be scheduled again.
		 */
		(void)k_work_cancel_delayable(&connect_work);
		break;
	case CLOUD_EVT_READY:
		printk("CLOUD_EVT_READY\n");
		display_state = LEDS_CLOUD_CONNECTED;
		sensors_init();
		atomic_set(&send_data_enable, 1);
		break;
	case CLOUD_EVT_DISCONNECTED:
		printk("CLOUD_EVT_DISCONNECTED\n");
		cloud_connected = false;
		k_work_reschedule(&connect_work, K_NO_WAIT);
		break;
	case CLOUD_EVT_ERROR:
		printk("CLOUD_EVT_ERROR\n");
		atomic_set(&send_data_enable, 0);
		display_state = LEDS_ERROR;
		break;
	case CLOUD_EVT_DATA_SENT:
		printk("CLOUD_EVT_DATA_SENT\n");
		break;
	case CLOUD_EVT_DATA_RECEIVED:
		printk("CLOUD_EVT_DATA_RECEIVED\n");
		break;
	case CLOUD_EVT_PAIR_REQUEST:
		printk("CLOUD_EVT_PAIR_REQUEST\n");
		break;
	case CLOUD_EVT_PAIR_DONE:
		printk("CLOUD_EVT_PAIR_DONE\n");
		break;
	case CLOUD_EVT_FOTA_DONE:
		printk("CLOUD_EVT_FOTA_DONE\n");
		break;
	case CLOUD_EVT_FOTA_ERROR:
		printk("CLOUD_EVT_FOTA_ERROR\n");
		break;
	default:
		printk("Unknown cloud event type: %d\n", evt->type);
		break;
	}
}

/**@brief Connect to Cloud, */
static void cloud_connect_work(struct k_work *work)
{
	int err;

	ARG_UNUSED(work);

	if (cloud_connected) {
		return;
	}

	err = cloud_connect(cloud_backend);

	if (err) {
		printk("Failed to connect to cloud: %d\n", err);
		printk("Retrying in 90 seconds");
		k_work_schedule(&connect_work, RETRY_CONNECT_WAIT);
	}

	display_state = LEDS_CLOUD_CONNECTING;
	k_work_schedule(&connect_work, RETRY_CONNECT_WAIT);
}

/**@brief Callback for button events from the DK buttons and LEDs library. */
static void button_handler(uint32_t buttons, uint32_t has_changed)
{
	printk("button_handler: button 1: %u, button 2: %u "
	       "switch 1: %u, switch 2: %u\n",
	       (bool)(buttons & BUTTON_1), (bool)(buttons & BUTTON_2), (bool)(buttons & SWITCH_1),
	       (bool)(buttons & SWITCH_2));
}

/**@brief Initializes and submits delayed work. */
static void work_init(void)
{
	k_work_init_delayable(&leds_update_work, leds_update);
	k_work_init_delayable(&connect_work, cloud_connect_work);
	k_work_schedule(&leds_update_work, LEDS_UPDATE_INTERVAL);
}

/**@brief Configures modem to provide LTE link. Blocks until link is
 * successfully established.
 */
static void modem_configure(void)
{
	display_state = LEDS_LTE_CONNECTING;

	if (IS_ENABLED(CONFIG_LTE_AUTO_INIT_AND_CONNECT)) {
		/* Do nothing, modem is already turned on and connected. */
	} else {
		int err;

		printk("Establishing LTE link (this may take some time) ...\n");
		err = lte_lc_init_and_connect();
		__ASSERT(err == 0, "LTE link could not be established.");
		display_state = LEDS_LTE_CONNECTED;
	}
}

/**@brief Initializes the sensors that are used by the application. */
static void sensors_init(void)
{
	int err;
	const struct device *gps_dev = device_get_binding(CONFIG_GPS_DEV_NAME);
	struct gps_config gps_cfg = {
		.nav_mode = GPS_NAV_MODE_PERIODIC,
		.interval = CONFIG_GPS_SEARCH_INTERVAL,
		.timeout = CONFIG_GPS_SEARCH_TIMEOUT,
	};

	if (gps_dev == NULL) {
		printk("Could not get %s device\n", CONFIG_GPS_DEV_NAME);
		return;
	}

	err = gps_init(gps_dev, gps_handler);
	if (err) {
		printk("Could not initialize GPS, error: %d\n", err);
		return;
	}

	printk("GPS initialized\n");

	err = gps_start(gps_dev, &gps_cfg);
	if (err) {
		printk("Failed to start GPS, error: %d\n", err);
		return;
	}

	printk("GPS started with interval %d seconds, and timeout %d seconds\n",
	       CONFIG_GPS_SEARCH_INTERVAL, CONFIG_GPS_SEARCH_TIMEOUT);
}

/**@brief Initializes buttons and LEDs, using the DK buttons and LEDs
 * library.
 */
static void buttons_leds_init(void)
{
	int err;

	err = dk_buttons_init(button_handler);
	if (err) {
		printk("Could not initialize buttons, err code: %d\n", err);
	}

	err = dk_leds_init();
	if (err) {
		printk("Could not initialize leds, err code: %d\n", err);
	}

	err = dk_set_leds_state(0x00, DK_ALL_LEDS_MSK);
	if (err) {
		printk("Could not set leds state, err code: %d\n", err);
	}
}

void main(void)
{
	int err;

	printk("LTE Sensor Gateway sample started\n");

	buttons_leds_init();
	ble_init();

	cloud_backend = cloud_get_binding(CONFIG_CLOUD_BACKEND);
	err = cloud_init(cloud_backend, cloud_event_handler);
	if (err) {
		printk("Cloud backend could not be initialized, error: %d", err);
	}
	modem_configure();
	cloud_connect(cloud_backend);

	work_init();

	while (true) {
		cloud_ping(cloud_backend);
		send_aggregated_data();
		k_sleep(K_MSEC(10));
	}
}
