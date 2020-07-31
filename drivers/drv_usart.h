/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-09     MurphyZhao        first version
 */

#ifndef __DRV_USART_H__
#define __DRV_USART_H__

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>

#include "hal_uart.h"
#include "hal_platform.h"

/* uart device dirver class */
ALIGN(4)
struct mt2625_uart
{
    const char *name;
    IRQn_Type irq_type;
    hal_uart_port_t uart_port;
    hal_gpio_pin_t uart_tx_pin;
    hal_gpio_pin_t uart_rx_pin;

    uint8_t uart_tx_pinmux;
    uint8_t uart_rx_pinmux;

#ifdef RT_SERIAL_USING_DMA
    struct
    {
        uint8_t using_rx_dma_mode;
        rt_size_t last_index;
    } dma;
#endif

    struct rt_serial_device serial;
};

int rt_hw_usart_init(void);

#endif  /* __DRV_USART_H__ */
