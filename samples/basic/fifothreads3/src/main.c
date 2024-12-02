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

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7



struct data_item_t {
	void *fifo_reserved; /* 1st word reserved for use by fifo */
	uint32_t value;
};
K_FIFO_DEFINE(fifo);

struct data_item_t tx1_data;
struct data_item_t *rx1_data;
struct data_item_t *rx2_data;

void writing1(void)
{
	while (1) {
		tx1_data.value = 10;
		k_fifo_put(&fifo, &tx1_data);
		k_msleep(1000);
	}
}
void reading1(void) 
{
	while (1) {
		rx_data = k_fifo_get(&fifo, K_FOREVER);
		k_msleep(1000);
		printk("Valor lido %d", rx1_data->value);
	}
}

void reading2(void) 
{
	while (1) {
		rx_data = k_fifo_get(&fifo, K_FOREVER);
		k_msleep(1000);
		printk("Valor lido %d", rx2_data->value);
	}
}


K_THREAD_DEFINE(writing1_id, STACKSIZE, writing1, NULL, NULL, NULL,
		PRIORITY, 0, 0);
K_THREAD_DEFINE(reading1_id, STACKSIZE, reading1, NULL, NULL, NULL,
		PRIORITY, 0, 0);
K_THREAD_DEFINE(reading2_id, STACKSIZE, reading2, NULL, NULL, NULL,
		PRIORITY, 0, 0);
