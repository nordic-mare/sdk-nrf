#include <zephyr.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_sense_led, CONFIG_UI_LOG_LEVEL);

#define GPIO_SENSE_RED_NODE_ID      DT_NODELABEL(sense_red_led)
#define GPIO_SENSE_GREEN_NODE_ID    DT_NODELABEL(sense_green_led)
#define GPIO_SENSE_BLUE_NODE_ID     DT_NODELABEL(sense_blue_led)

#define GPIO_NAME  DT_GPIO_LABEL(GPIO_SENSE_RED_NODE_ID, gpios)

#define RED_SENSE_LED_GPIO_PIN      DT_GPIO_PIN(GPIO_SENSE_RED_NODE_ID, gpios)
#define GREEN_SENSE_LED_GPIO_PIN    DT_GPIO_PIN(GPIO_SENSE_GREEN_NODE_ID, gpios)
#define BLUE_SENSE_LED_GPIO_PIN	    DT_GPIO_PIN(GPIO_SENSE_BLUE_NODE_ID, gpios)

#define SENSE_LED_GPIO_FLAGS	    DT_GPIO_FLAGS(GPIO_SENSE_RED_NODE_ID, gpios)

static const struct device *gpio_dev;

int ui_sense_led_on_off(bool new_state)
{
    int ret;
    ret = gpio_pin_set(gpio_dev, RED_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set red sense LED");
        return ret;
    }
    ret = gpio_pin_set(gpio_dev, GREEN_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set green sense LED");
        return ret;
    }
    ret = gpio_pin_set(gpio_dev, BLUE_SENSE_LED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Could not set blue sense LED");
        return ret;
    }
    return 0;
}


int ui_sense_led_init(void)
{
	int ret;

	gpio_dev = device_get_binding(GPIO_NAME);
	if (!gpio_dev) {
        LOG_ERR("No GPIO device found");
		return -ENODEV;
	}

	ret = gpio_pin_configure(gpio_dev, RED_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
        LOG_ERR("Could not configure red sense LED");
		return ret;
	}
    ret = gpio_pin_configure(gpio_dev, GREEN_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
        LOG_ERR("Could not configure green sense LED");
		return ret;
	}
    ret = gpio_pin_configure(gpio_dev, BLUE_SENSE_LED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
        LOG_ERR("Could not configure blue sense LED");
		return ret;
	}

	return 0;
}