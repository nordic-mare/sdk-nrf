#ifndef SENSOR_EVENT_H__
#define SENSOR_EVENT_H__

#include <zephyr.h>
#include <net/lwm2m.h>
#include <event_manager.h>

enum sensor_type {
    LightSensor,
    ColourSensor,
    TemperatureSensor,
    HumiditySensor,
    PressureSensor,
    GasResistanceSensor
};

struct sensor_event {
    struct event_header header;

    enum sensor_type type;
    uint32_t unsigned_val;
    int32_t float_val1;
    int32_t float_val2;
};

EVENT_TYPE_DECLARE(sensor_event);

#endif