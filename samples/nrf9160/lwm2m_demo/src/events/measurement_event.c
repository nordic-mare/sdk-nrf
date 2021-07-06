#include "measurement_event.h"

#include <stdio.h>

static int log_measurement_event(const struct event_header *eh, char *buf,
                            size_t buf_len)
{
    struct measurement_event *event = cast_measurement_event(eh);

    return snprintf(buf, buf_len, 
            STRINGIFY(event->type) " measurement event: unsigned_val=%u, float_val1=%d, float_val2=%d", 
            event->unsigned_val, event->float_val1, event->float_val2);
}


EVENT_TYPE_DEFINE(
    measurement_event,
    true,
    log_measurement_event,
    NULL
);