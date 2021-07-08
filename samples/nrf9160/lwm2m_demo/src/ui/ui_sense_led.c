#include <zephyr.h>
#include <drivers/gpio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ui_sense_led, CONFIG_UI_LOG_LEVEL);

#define SENSE_RED_GPIO_NODE_ID      DT_NODELABEL(sense_red_led)
#define SENSE_GREEN_GPIO_NODE_ID    DT_NODELABEL(sense_green_led)
#define SENSE_BLUE_GPIO_NODE_ID     DT_NODELABEL(sense_blue_led)

#define SENSE_LED_GPIO_NAME         DT_GPIO_LABEL(SENSE_RED_GPIO_NODE_ID, gpios)

#define SENSE_RED_GPIO_PIN          DT_GPIO_PIN(SENSE_RED_GPIO_NODE_ID, gpios)
#define SENSE_GREEN_GPIO_PIN        DT_GPIO_PIN(SENSE_GREEN_GPIO_NODE_ID, gpios)
#define SENSE_BLUE_GPIO_PIN	        DT_GPIO_PIN(SENSE_BLUE_GPIO_NODE_ID, gpios)

#define SENSE_LED_GPIO_FLAGS	    DT_GPIO_FLAGS(SENSE_RED_GPIO_NODE_ID, gpios)

static const struct device *sense_led_gpio_dev;


int ui_sense_led_on_off(bool new_state)
{
    int ret;
    ret = gpio_pin_set(sense_led_gpio_dev, SENSE_RED_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Error %d: set red pin failed", ret);
        return ret;
    }
    ret = gpio_pin_set(sense_led_gpio_dev, SENSE_GREEN_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Error %d: set green pin failed", ret);
        return ret;
    }
    ret = gpio_pin_set(sense_led_gpio_dev, SENSE_BLUE_GPIO_PIN, new_state);
    if (ret) {
        LOG_ERR("Error %d: set blue pin failed", ret);
        return ret;
    }
    return 0;
}


int ui_sense_led_init(void)
{
	int ret;

	sense_led_gpio_dev = device_get_binding(SENSE_LED_GPIO_NAME);
	if (!sense_led_gpio_dev) {
        LOG_ERR("Error %d: could not bind to GPIO device", -ENODEV);
		return -ENODEV;
	}

	ret = gpio_pin_configure(sense_led_gpio_dev, SENSE_RED_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
        LOG_ERR("Error %d: configure red pin failed", ret);
		return ret;
	}
    ret = gpio_pin_configure(sense_led_gpio_dev, SENSE_GREEN_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
    if (ret) {
        LOG_ERR("Error %d: configure green pin failed", ret);
		return ret;
	}
    ret = gpio_pin_configure(sense_led_gpio_dev, SENSE_BLUE_GPIO_PIN, 
                    SENSE_LED_GPIO_FLAGS | GPIO_OUTPUT_INACTIVE);
	if (ret) {
        LOG_ERR("Error %d: configure blue pin failed", ret);
		return ret;
	}

	return 0;
}