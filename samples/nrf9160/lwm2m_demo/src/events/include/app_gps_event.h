#ifndef APP_GPS_EVENT_H__
#define APP_GPS_EVENT_H__

#include <zephyr.h>
#include <drivers/gps.h>
#include <event_manager.h>

struct app_gps_event {
    struct event_header header;

    struct gps_pvt pvt;
};

EVENT_TYPE_DECLARE(app_gps_event);
#endif