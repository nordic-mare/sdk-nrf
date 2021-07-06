#ifndef MEASUREMENT_EVENT_H__
#define MEASUREMENT_EVENT_H__

#include <zephyr.h>
#include <event_manager.h>

enum measurement_type {
    LightMeasurement,
    ColourMeasurement,
    TempeartureMeasurement,
    HumidityMeasurement,
    PressureMeasurement,
    GasResistanceMeasurement
};

struct measurement_event {
    struct event_header header;

    enum measurement_type type;
    uint32_t unsigned_val;
    int32_t float_val1;
    int32_t float_val2;
};

EVENT_TYPE_DECLARE(measurement_event);

#endif