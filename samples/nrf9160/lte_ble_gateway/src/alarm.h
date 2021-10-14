/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _ALARM_H_
#define _ALARM_H_

struct cloud_sensor_data {
	char *type;
	char *data;
    uint16_t tag;
	uint8_t length;
};

void alarm(void);
void send_aggregated_data(void);

#endif /* _ALARM_H_ */
