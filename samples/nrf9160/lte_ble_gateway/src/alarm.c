/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <string.h>
#include <stdbool.h>
#include <sys/printk.h>
#include <net/cloud.h>

#include "alarm.h"

#include "aggregator.h"

static bool alarm_pending;

extern void cloud_update(struct cloud_sensor_data cloud_data);

char *orientation_strings[] = { "LEFT", "NORMAL", "RIGHT", "UPSIDE_DOWN" };

void alarm(void)
{
	alarm_pending = true;
}

void send_aggregated_data(void)
{
	struct sensor_data aggregator_data;
	struct cloud_sensor_data cloud_data;

	if (!alarm_pending) {
		return;
	}

	alarm_pending = false;

	printk("Alarm triggered !\n");
	while (true) {
		if (aggregator_get(&aggregator_data) == -ENODATA) {
			break;
		}
		switch (aggregator_data.type) {
		case THINGY_ORIENTATION:
			printk("%d] Sending FLIP data.\n", aggregator_element_count_get());
			if (aggregator_data.length != 1 ||
			    aggregator_data.data[0] >= ARRAY_SIZE(orientation_strings)) {
				printk("Unexpected FLIP data format, dropping\n");
				continue;
			}
			cloud_data.type = "FLIP";
			cloud_data.data = orientation_strings[aggregator_data.data[0]];
			cloud_data.length =
				strlen(orientation_strings[aggregator_data.data[0]]) - 1;
			cloud_update(cloud_data);
			break;

		case GPS_POSITION:
			printk("%d] Sending GPS data.\n", aggregator_element_count_get());
			cloud_data.type = "GPS";
			cloud_data.data = &aggregator_data.data[4];
			cloud_data.length = aggregator_data.length;
			cloud_update(cloud_data);
			break;

		default:
			printk("Unsupported data type from aggregator: %d.\n",
			       aggregator_data.type);
			continue;
		}
	}
}
