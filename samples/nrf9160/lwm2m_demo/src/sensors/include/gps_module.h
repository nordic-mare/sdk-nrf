/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef GPS_MODULE_H__
#define GPS_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

int initialise_gps(void);

int start_gps_search(void);

#endif
