/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-09     MurphyZhao        first version
 */

#include "rtthread.h"
#include <rtdevice.h>
#include <stdint.h>

#include "board.h"

#ifdef NB_GPIO_DEMO

#define LOG_TAG              "demo.gpio"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define TEST_PIN_NUM_0 (NB_PIN_21) /* output, sys state led */
#define TEST_PIN_NUM_1 (NB_PIN_34) /* input  pull up, falling interrupt, GPIO0 */
#define TEST_PIN_NUM_2 (NB_PIN_35) /* input  pull down, GPIO1 */

#define TEST_GPIO_THR_PRI   (230u)
#define TEST_GPIO_THR_STACK (1024u)
#define TEST_CNT            (50u)

static void pin_1_falling_interrupt_cb(void *arg)
{
    uint16_t i;
    rt_pin_irq_enable(TEST_PIN_NUM_1, PIN_IRQ_DISABLE);
    i = *((uint16_t *)arg);
    rt_kprintf("In pin1 falling interrupt callback. arg: %d\r\n", i);
    rt_pin_irq_enable(TEST_PIN_NUM_1, PIN_IRQ_ENABLE);
}

static void test_gpio_thread(void *arg)
{
    uint16_t i = 0;

    rt_pin_mode(TEST_PIN_NUM_0, PIN_MODE_OUTPUT);
    rt_pin_write(TEST_PIN_NUM_0, PIN_LOW);

    rt_pin_mode(TEST_PIN_NUM_1, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(TEST_PIN_NUM_1, PIN_IRQ_MODE_FALLING, pin_1_falling_interrupt_cb, &i);
    rt_pin_irq_enable(TEST_PIN_NUM_1, PIN_IRQ_ENABLE);

    rt_pin_mode(TEST_PIN_NUM_2, PIN_MODE_INPUT_PULLDOWN);

    while (1)
    {
        rt_pin_write(TEST_PIN_NUM_0, 0);
        LOG_I("[%d] pin0: %d; pin1: %d; pin2: %d",
                i, 
                rt_pin_read(TEST_PIN_NUM_0), 
                rt_pin_read(TEST_PIN_NUM_1), 
                rt_pin_read(TEST_PIN_NUM_2));
        rt_thread_mdelay(1000);

        rt_pin_write(TEST_PIN_NUM_0, 1);
        LOG_I("[%d] pin0: %d; pin1: %d; pin2: %d", 
                i,
                rt_pin_read(TEST_PIN_NUM_0), 
                rt_pin_read(TEST_PIN_NUM_1), 
                rt_pin_read(TEST_PIN_NUM_2));
        rt_thread_mdelay(1000);

        i ++;
        if (i == TEST_CNT)
        {
            break;
        }
    }
}

static int mt2625_gpio_sample(void *arg)
{
    rt_thread_t tid = rt_thread_create("gpio_demo", test_gpio_thread, 
            NULL, TEST_GPIO_THR_STACK, TEST_GPIO_THR_PRI, 5);
    if (tid)
        rt_thread_startup(tid);
    else
    {
        LOG_E("test thread create failed.");
        return -1;
    }
    return 0;
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(mt2625_gpio_sample, demo_gpio, gpio driver sample);
#endif /* FINSH_USING_MSH */

#endif /* NB_GPIO_DEMO */
