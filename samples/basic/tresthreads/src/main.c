/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/__assert.h>
#include <string.h>
#include <zephyr/sys/util.h>

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

#define LED0_NODE DT_ALIAS(led0)

#define SLEEP_TIME_MS   1

#if !DT_NODE_HAS_STATUS_OKAY(LED0_NODE)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#define SW0_NODE    DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
                                  {0});
static struct gpio_callback button_cb_data;

static struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios,
                             {0});
                    

K_MUTEX_DEFINE(my_mutex_blink);
K_MUTEX_DEFINE(my_mutex_buttom);

struct led {
    struct gpio_dt_spec spec;
    uint8_t num;
};

static const struct led led0 = {
    .spec = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0}),
    .num = 0,
};

void button_pressed(const struct device *dev, struct gpio_callback *cb,
            uint32_t pins)
{
    printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
}

void blink0(void)
{
	const struct led *led = &led0;
	int sleep_ms = 3000;
	int id = 0;


    const struct gpio_dt_spec *spec = &led->spec;
    int cnt = 1;
    int ret;

    if (!device_is_ready(spec->port)) {
        printk("Error: %s device is not ready\n", spec->port->name);
        return;
    }

    ret = gpio_pin_configure_dt(spec, GPIO_OUTPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure pin %d (LED '%d')\n",
            ret, spec->pin, led->num);
        return;
    }


        while (1) {
            if(k_mutex_lock(&my_mutex_blink, K_FOREVER) == 0) {
                gpio_pin_set(spec->port, spec->pin, cnt % 2);
				printk("lol");
                k_msleep(sleep_ms);
                cnt++;
                k_mutex_unlock(&my_mutex_blink);

				gpio_pin_set(spec->port, spec->pin, cnt % 2);
				cnt++;

				printk("lol2");
				k_msleep(sleep_ms);
            }
        }

}

void buttom(void)
{
int ret;

    if (!gpio_is_ready_dt(&button)) {
        printk("Error: button device %s is not ready\n",
               button.port->name);
        return 0;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n",
               ret, button.port->name, button.pin);
        return 0;
    }

    ret = gpio_pin_interrupt_configure_dt(&button,
                          GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n",
            ret, button.port->name, button.pin);
        return 0;
    }

    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    printk("Set up button at %s pin %d\n", button.port->name, button.pin);

    if (led.port && !gpio_is_ready_dt(&led)) {
        printk("Error %d: LED device %s is not ready; ignoring it\n",
               ret, led.port->name);
        led.port = NULL;
    }
    if (led.port) {
        ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT);
        if (ret != 0) {
            printk("Error %d: failed to configure LED device %s pin %d\n",
                   ret, led.port->name, led.pin);
            led.port = NULL;
        } else {
            printk("Set up LED at %s pin %d\n", led.port->name, led.pin);
        }
    }

    printk("Press the button\n");
    if (led.port) {
        while (1) {
            /* If we have an LED, match its state to the button's. */
            int val = gpio_pin_get_dt(&button);
            if(k_mutex_lock(&my_mutex_buttom, K_FOREVER) == 0 && val>0) {
                gpio_pin_set_dt(&led, val);
				printk("bora");
            } else if (val==0) {
                k_mutex_unlock(&my_mutex_buttom);
                gpio_pin_set_dt(&led, val);
				printk("bora2");
            }               
            k_msleep(SLEEP_TIME_MS);
        }
    return 0;
    }
}
void both(void) {
	while (1) {
		if(k_mutex_lock(&my_mutex_blink, K_FOREVER) == 0 ){
			printk("pegou pato1");
			if(k_mutex_lock(&my_mutex_buttom, K_MSEC(100)) == 0) {
				printk("AAAAAAAAA");
				k_msleep(1000);
				k_mutex_unlock(&my_mutex_blink);
			    k_mutex_unlock(&my_mutex_buttom);
			}else{ 
				k_mutex_unlock(&my_mutex_blink);
			}	
		//k_mutex_unlock(&my_mutex_buttom);
	}
	}
}
K_THREAD_DEFINE(blink0_id, STACKSIZE, blink0, NULL, NULL, NULL,
        PRIORITY, 0, 0);

K_THREAD_DEFINE(buttom_id, STACKSIZE, buttom, NULL, NULL, NULL,
        PRIORITY, 0, 0);

K_THREAD_DEFINE(both_id, STACKSIZE, both, NULL, NULL, NULL, PRIORITY, 0, 0);
