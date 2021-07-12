/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <drivers/pwm.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_buzzer, CONFIG_UI_LOG_LEVEL);

#define BUZZER_PWM_NODE			DT_ALIAS(buzzer_pwm)
#define BUZZER_PWM_NAME			DT_LABEL(BUZZER_PWM_NODE)
#define BUZZER_PWM_PIN			DT_PROP(BUZZER_PWM_NODE, ch0_pin)
#define BUZZER_PWM_FLAGS        DT_PWMS_FLAGS(BUZZER_PWM_NODE)

#define PERIOD(freq)				(USEC_PER_SEC / freq)
#define PULSE_WIDTH(freq, duty)		(PERIOD(freq) * (uint32_t)duty / 100U)

#define FREQUENCE_MAX				10000U
#define DUTYCYCLE_MAX				50U

static const struct device *buzzer_pwm_dev;
static bool state;
static uint32_t frequency;
static uint8_t dutycycle;

int ui_buzzer_on_off(bool new_state)
{
	int ret;

	state = new_state;

	LOG_DBG("Frequency: %u", frequency);

	if (frequency == 0) {
		ret = pwm_pin_set_usec(buzzer_pwm_dev, BUZZER_PWM_PIN,
			UINT32_MAX, 0,
			BUZZER_PWM_FLAGS);
	}
	else {
		LOG_DBG("ON/OFF period: %u", PERIOD(frequency));
		LOG_DBG("ON/OFF pulse width: %u", PULSE_WIDTH(frequency * state, dutycycle));
		ret = pwm_pin_set_usec(buzzer_pwm_dev, BUZZER_PWM_PIN,
			PERIOD(frequency), PULSE_WIDTH(frequency * state, dutycycle),
			BUZZER_PWM_FLAGS);
	}
	
	if (ret) {
		LOG_ERR("Error %d: set pwm pin failed", ret);
		return ret;
	}

	return 0;
}

int ui_buzzer_set_frequency(uint32_t freq)
{
	int ret;

	if (freq > FREQUENCE_MAX) {
		LOG_ERR("Error %d: frequency too high", -EINVAL);
		return -EINVAL;
	}

	frequency = freq;

	if (state) {
		ret = ui_buzzer_on_off(state);
		if (ret) {
			LOG_ERR("Error %d: set buzzer on/off failed", ret);
			return ret;
		}
	}
	
	return 0;
}

int ui_buzzer_set_dutycycle(uint8_t duty)
{
	int ret;

	if (duty > DUTYCYCLE_MAX) {
		LOG_ERR("Error %d: dutycycle too large", -EINVAL);
		return -EINVAL;
	}

	dutycycle = duty;

	if (state) {
		ret = ui_buzzer_on_off(state);
		if (ret) {
			LOG_ERR("Error %d: set buzzer on/off failed", ret);
			return ret;
		}
	}

	return 0;
}

int ui_buzzer_init(void)
{
	buzzer_pwm_dev = device_get_binding(BUZZER_PWM_NAME);
	if (!buzzer_pwm_dev) {
		LOG_ERR("Error %d: could not bind to LED GPIO device", -ENODEV);
		return -ENODEV;
	}

	frequency = 440;
	dutycycle = 50;

	return 0;
}

// Kept temporarily for inspiration. Safe to remove
static int pwm_out(uint32_t frequency, uint8_t intensity)
{
	static uint32_t prev_period;
	uint32_t period = (frequency > 0) ? USEC_PER_SEC / frequency : 0;
	uint32_t duty_cycle = 0; //(intensity == 0) ? 0 :
		//period / intensity_to_duty_cycle_divisor();

	/* Applying workaround due to limitations in PWM driver that doesn't
	 * allow changing period while PWM is running. Setting pulse to 0
	 * disables the PWM, but not before the current period is finished.
	 */
	if (prev_period) {
		pwm_pin_set_usec(buzzer_pwm_dev, BUZZER_PWM_PIN,
				 prev_period, 0, 0);
		k_sleep(K_MSEC(MAX((prev_period / USEC_PER_MSEC), 1)));
	}

	prev_period = period;

	return pwm_pin_set_usec(buzzer_pwm_dev, BUZZER_PWM_PIN,
				period, duty_cycle, 0);
}