#ifndef MEASUREMENT_EVENT_H__
#define MEASUREMENT_EVENT_H__

#include <zephyr.h>
#include <net/lwm2m.h>
#include <event_manager.h>

enum measurement_type {
    LightMeasurement,
    ColourMeasurement,
    TemperatureMeasurement,
    HumidityMeasurement,
    PressureMeasurement,
    GasResistanceMeasurement
};

struct measurement_event {
    struct event_header header;

    enum measurement_type type;
    uint32_t unsigned_val;
    float32_value_t float_val;
};

EVENT_TYPE_DECLARE(measurement_event);

#endif