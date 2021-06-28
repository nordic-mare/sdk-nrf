#include <zephyr.h>
#include <drivers/gpio.h>

#define GPIO_SENSE_RED_NODE_ID      DT_NODELABEL(sense_red_led)
#define GPIO_SENSE_GREEN_NODE_ID    DT_NODELABEL(sense_green_led)
#define GPIO_SENSE_BLUE_NODE_ID     DT_NODELABEL(sense_blue_led)

#define GPIO_NAME  DT_GPIO_LABEL(GPIO_SENSE_RED_NODE_ID, gpios)

#define RED_SENSE_LED_GPIO_PIN      DT_GPIO_PIN(GPIO_SENSE_RED_NODE_ID, gpios)
#define GREEN_SENSE_LED_GPIO_PIN    DT_GPIO_PIN(GPIO_SENSE_GREEN_NODE_ID, gpios)
#define BLUE_SENSE_LED_GPIO_PIN	    DT_GPIO_PIN(GPIO_SENSE_BLUE_NODE_ID, gpios)

#define SENSE_LED_GPIO_FLAGS	    DT_GPIO_FLAGS(GPIO_SENSE_RED_NODE_ID, gpios)

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_sense_led, CONFIG_UI_LOG_LEVEL);

static const struct device *sense_led_dev;

int ui_sense_led_on_off(bool new_state)
{
    int ret;
    ret = gpio_pin_set(sense_led_dev, RED_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set red sense led");
        return ret;
    }
    ret = gpio_pin_set(sense_led_dev, GREEN_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set green sense led");
        return ret;
    }
    ret = gpio_pin_set(sense_led_dev, BLUE_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set blue sense led");
        return ret;
    }
    return 0;
}


int ui_sense_led_init(void)
{
	int ret;

	sense_led_dev = device_get_binding(GPIO_NAME);
	if (!sense_led_dev) {
		return -ENODEV;
	}

	ret = gpio_pin_configure(sense_led_dev, RED_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
		return ret;
	}
    ret = gpio_pin_configure(sense_led_dev, GREEN_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
		return ret;
	}
    ret = gpio_pin_configure(sense_led_dev, BLUE_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
		return ret;
	}

	return 0;
}