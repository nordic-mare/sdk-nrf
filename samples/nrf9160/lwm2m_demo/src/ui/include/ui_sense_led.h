#ifndef UI_SENSE_LED_H__
#define UI_SENSE_LED_H__

#include <zephyr.h>

int ui_sense_led_init(void);

int ui_sense_led_on_off(bool new_state);

#endif