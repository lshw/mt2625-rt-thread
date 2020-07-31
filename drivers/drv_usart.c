/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-09     MurphyZhao        first version
 */
 
#include "board.h"
#include "drv_usart.h"

/* MT2625 */
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_uart.h"
#include "hal_gpio.h"

#ifdef RT_USING_SERIAL

#if defined BSP_USING_UART0 || defined BSP_USING_UART1 || defined BSP_USING_UART2 || BSP_USING_UART3

static struct mt2625_uart uart_obj_sets[] = 
{
#ifdef BSP_USING_UART0
    {
        .name = "uart0",
        .irq_type = UART0_IRQn,
        .uart_port = HAL_UART_0,
        .uart_tx_pinmux = HAL_GPIO_5_UART0_TXD,
        .uart_rx_pinmux = HAL_GPIO_2_UART0_RXD,
        .uart_tx_pin = HAL_GPIO_5,
        .uart_rx_pin = HAL_GPIO_2,
#ifdef BSP_UART0_RX_USING_DMA
        .dma.using_rx_dma_mode = 1
#endif
    },
#endif

#ifdef BSP_USING_UART1
    {
        .name = "uart1",
        .irq_type = UART1_IRQn,
        .uart_port = HAL_UART_1,
        .uart_tx_pinmux = HAL_GPIO_17_UART1_TXD,
        .uart_rx_pinmux = HAL_GPIO_16_UART1_RXD,
        .uart_tx_pin = HAL_GPIO_17,
        .uart_rx_pin = HAL_GPIO_16,
#ifdef BSP_UART1_RX_USING_DMA
        .dma.using_rx_dma_mode = 1
#endif
    },
#endif

#ifdef BSP_USING_UART2
    {
        .name = "uart2",
        .irq_type = UART2_IRQn,
        .uart_port = HAL_UART_2,
        .uart_tx_pinmux = HAL_GPIO_4_UART2_TXD,
        .uart_rx_pinmux = HAL_GPIO_3_UART2_RXD,
        .uart_tx_pin = HAL_GPIO_4,
        .uart_rx_pin = HAL_GPIO_3,
#ifdef BSP_UART2_RX_USING_DMA
        .dma.using_rx_dma_mode = 1
#endif
    },
#endif

#ifdef BSP_USING_UART3
    {
        .name = "uart3",
        .irq_type = UART3_IRQn,
        .uart_port = HAL_UART_3,
        .uart_tx_pinmux = HAL_GPIO_34_UART2_TXD,
        .uart_rx_pinmux = HAL_GPIO_33_UART2_RXD,
        .uart_tx_pin = HAL_GPIO_34,
        .uart_rx_pin = HAL_GPIO_33,
#ifdef BSP_UART3_RX_USING_DMA
        .dma.using_rx_dma_mode = 1
#endif
    },
#endif
};

static struct rt_serial_device *get_serial_dev(hal_uart_port_t uart_port)
{
    for (int i = 0; i < (sizeof(uart_obj_sets)/sizeof(uart_obj_sets[0])); i ++)
    {
        if (uart_obj_sets[i].uart_port == uart_port)
        {
            return &(uart_obj_sets[i].serial);
        }
    }
    return NULL;
}

#ifdef BSP_USING_UART0
static void mt2625_uart0_idle_rx(void)
{
    rt_hw_serial_isr(get_serial_dev(HAL_UART_0), RT_SERIAL_EVENT_RX_IND);
}
#endif

#ifdef BSP_USING_UART1
static void mt2625_uart1_idle_rx(void)
{
    rt_hw_serial_isr(get_serial_dev(HAL_UART_1), RT_SERIAL_EVENT_RX_IND);
}
#endif

#ifdef BSP_USING_UART2
static void mt2625_uart2_idle_rx(void)
{
    rt_hw_serial_isr(get_serial_dev(HAL_UART_2), RT_SERIAL_EVENT_RX_IND);
}
#endif

#ifdef BSP_USING_UART3
static void mt2625_uart3_idle_rx(void)
{
    rt_hw_serial_isr(get_serial_dev(HAL_UART_3), RT_SERIAL_EVENT_RX_IND);
}
#endif

static rt_err_t mt2625_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    struct mt2625_uart *uart = (struct mt2625_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    /* init device uart */
    hal_gpio_init(uart->uart_tx_pin);
    hal_pinmux_set_function(uart->uart_tx_pin, uart->uart_tx_pinmux);
    hal_gpio_init(uart->uart_rx_pin);
    hal_pinmux_set_function(uart->uart_rx_pin, uart->uart_rx_pinmux);

    hal_uart_config_t uart_config;

    switch (cfg->baud_rate)
    {
        case BAUD_RATE_9600:
            uart_config.baudrate = HAL_UART_BAUDRATE_9600;
            break;
        case BAUD_RATE_115200:
            uart_config.baudrate = HAL_UART_BAUDRATE_115200;
            break;
        case BAUD_RATE_921600:
            uart_config.baudrate = HAL_UART_BAUDRATE_921600;
            break;
        default:
            uart_config.baudrate = HAL_UART_BAUDRATE_115200;
            break;
    }

    switch (cfg->data_bits)
    {
        case DATA_BITS_7:
            uart_config.word_length = HAL_UART_WORD_LENGTH_7;
            break;
        case DATA_BITS_8:
            uart_config.word_length = HAL_UART_WORD_LENGTH_8;
            break;
        default:
            uart_config.word_length = HAL_UART_WORD_LENGTH_8;
            break;
    }

    switch (cfg->stop_bits)
    {
        case STOP_BITS_1:
            uart_config.stop_bit = HAL_UART_STOP_BIT_1;
            break;
        case STOP_BITS_2:
            uart_config.stop_bit = HAL_UART_STOP_BIT_2;
            break;
        default:
            uart_config.stop_bit = HAL_UART_STOP_BIT_1;
            break;
    }

    switch (cfg->parity)
    {
        case PARITY_NONE:
            uart_config.parity = HAL_UART_PARITY_NONE;
            break;
        case PARITY_ODD:
            uart_config.parity = HAL_UART_PARITY_ODD;
            break;
        case PARITY_EVEN:
            uart_config.parity = HAL_UART_PARITY_EVEN;
            break;
        default:
            uart_config.parity = HAL_UART_PARITY_NONE;
            break;
    }

    if (hal_uart_init(uart->uart_port, &uart_config) != HAL_UART_STATUS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

#ifdef RT_SERIAL_USING_DMA
#include "hal_pdma_internal.h"

static void mt2625_uart_dma_irq_cb(hal_uart_callback_event_t event, void *user_data)
{
    RT_ASSERT(user_data != RT_NULL);
    struct rt_serial_device *serial = (struct rt_serial_device *)user_data;
    struct mt2625_uart *uart = (struct stm32_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    vdma_channel_t channel;
    rt_size_t recv_total_index, recv_len;
    uint32_t avail_count;
    rt_base_t level;

    if (HAL_UART_EVENT_READY_TO_READ == event)
    {
        channel = uart_port_to_dma_channel(uart->uart_port, 1);
        if (vdma_get_available_receive_bytes(channel, &avail_count) != VDMA_OK)
        {
            RT_ASSERT(0);
            return;
        }
        level = rt_hw_interrupt_disable();
        recv_total_index = avail_count; //serial->config.bufsz - __HAL_DMA_GET_COUNTER(&(uart->dma.handle));
        recv_len = recv_total_index - uart->dma.last_index;
        uart->dma.last_index = recv_total_index;
        rt_hw_interrupt_enable(level);

        if (recv_len)
        {
            rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
        }
    }
    else if (HAL_UART_EVENT_READY_TO_WRITE == event)
    {
        /* TODO */
        rt_kprintf("uart dma ready to write.\n");
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_TX_DMADONE);
    }
    else
    {
        rt_kprintf("No handler event:%d.\n", event);
    }
    
}

#include "memory_attribute.h"
#define MT2625_DMA_FIFO_TX_BUF_SIZE (64)
ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart0_dma_fifo_tx_buf[MT2625_DMA_FIFO_TX_BUF_SIZE];

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart1_dma_fifo_tx_buf[MT2625_DMA_FIFO_TX_BUF_SIZE];

ATTR_ZIDATA_IN_NONCACHED_RAM_4BYTE_ALIGN static uint8_t uart2_dma_fifo_tx_buf[MT2625_DMA_FIFO_TX_BUF_SIZE];

uint8_t *mt2625_dma_buf_sets[] = {uart0_dma_fifo_tx_buf, uart1_dma_fifo_tx_buf, uart2_dma_fifo_tx_buf};

static rt_err_t mt2625_dma_configure(struct rt_serial_device *serial)
{
    RT_ASSERT(serial != RT_NULL);
    struct mt2625_uart *uart = (struct stm32_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);
    struct rt_serial_rx_fifo *rx_fifo;
    hal_uart_dma_config_t dma_config;

    rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;

    /* fifo 缓冲区剩余可用空间阈值，用于触发 uart 的流控制系统 */
    dma_config.receive_vfifo_alert_size = 32;
    dma_config.receive_vfifo_buffer = (uint8_t *)rx_fifo->buffer;
    /* default 64 bytes */
    dma_config.receive_vfifo_buffer_size = serial->config.bufsz;
    /* fifo 接收缓冲区阈值，当缓冲区可用字节数超过该阈值时，触发 DMA 中断 */
    dma_config.receive_vfifo_threshold_size = 32;
    dma_config.send_vfifo_buffer = (uint8_t *)mt2625_dma_buf_sets[uart->uart_port];
    dma_config.send_vfifo_buffer_size = MT2625_DMA_FIFO_TX_BUF_SIZE;
    dma_config.send_vfifo_threshold_size = 8;

    hal_uart_set_dma(uart->uart_port, &dma_config);
    hal_uart_register_callback(uart->uart_port, mt2625_uart_dma_irq_cb, serial);
    return RT_EOK;
}
#endif

#define HAL_UART0_MASK (1 << 0)
#define HAL_UART1_MASK (1 << 1)
#define HAL_UART2_MASK (1 << 2)
#define HAL_UART3_MASK (1 << 3)

static rt_err_t mt2625_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    static uint8_t sethook = 0;
    RT_ASSERT(serial != RT_NULL);

    struct mt2625_uart *uart = (struct mt2625_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

#ifdef RT_SERIAL_USING_DMA
    rt_ubase_t ctrl_arg = (rt_ubase_t)arg;
#endif

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        switch (uart->uart_port)
        {
#ifdef BSP_USING_UART0
            case HAL_UART_0:
                if (sethook & HAL_UART0_MASK)
                {
                    rt_thread_idle_delhook(mt2625_uart0_idle_rx);
                    sethook &= ~HAL_UART0_MASK;
                }
                break;
#endif
#ifdef BSP_USING_UART1
            case HAL_UART_1:
                if (sethook & HAL_UART1_MASK)
                {
                    rt_thread_idle_delhook(mt2625_uart1_idle_rx);
                    sethook &= ~HAL_UART1_MASK;
                }
                break;
#endif
#ifdef BSP_USING_UART2
            case HAL_UART_2:
                if (sethook & HAL_UART2_MASK)
                {
                    rt_thread_idle_delhook(mt2625_uart2_idle_rx);
                    sethook &= ~HAL_UART2_MASK;
                }
                break;
#endif
#ifdef BSP_USING_UART3
            case HAL_UART_3:
                if (sethook & HAL_UART3_MASK)
                {
                    rt_thread_idle_delhook(mt2625_uart3_idle_rx);
                    sethook &= ~HAL_UART3_MASK;
                }
                break;
#endif
            default:
                break;
        }
        break;
    case RT_DEVICE_CTRL_SET_INT:
        switch (uart->uart_port)
        {
#ifdef BSP_USING_UART0
            case HAL_UART_0:
                if (!(sethook & HAL_UART0_MASK))
                {
                    rt_thread_idle_sethook(mt2625_uart0_idle_rx);
                    sethook |= HAL_UART0_MASK;
                }
                break;
#endif
#ifdef BSP_USING_UART1
            case HAL_UART_1:
                if (!(sethook & HAL_UART1_MASK))
                {
                    rt_thread_idle_sethook(mt2625_uart1_idle_rx);
                    sethook |= HAL_UART1_MASK;
                }
                break;
#endif
#ifdef BSP_USING_UART2
            case HAL_UART_2:
                if (!(sethook & HAL_UART2_MASK))
                {
                    rt_thread_idle_sethook(mt2625_uart2_idle_rx);
                    sethook |= HAL_UART2_MASK;
                }
                break;
#endif
#ifdef BSP_USING_UART3
            case HAL_UART_3:
                if (!(sethook & HAL_UART3_MASK))
                {
                    rt_thread_idle_sethook(mt2625_uart3_idle_rx);
                    sethook |= HAL_UART3_MASK;
                }
                break;
#endif
            default:
                break;
        }
        break;
#ifdef RT_SERIAL_USING_DMA
    case RT_DEVICE_CTRL_CONFIG:
        if (ctrl_arg == RT_DEVICE_FLAG_DMA_RX)
        {
            mt2625_dma_configure(serial);
        }
        break;
#endif
    }
    return RT_EOK;
}

static int mt2625_putc(struct rt_serial_device *serial, char c)
{
    RT_ASSERT(serial != RT_NULL);
    struct mt2625_uart *uart = (struct mt2625_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    hal_uart_put_char(uart->uart_port, c);
    return 1;
}

static int mt2625_getc(struct rt_serial_device *serial)
{
    uint32_t c;
    RT_ASSERT(serial != RT_NULL);
    struct mt2625_uart *uart = (struct mt2625_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    c = hal_uart_get_char_unblocking(uart->uart_port);
    return c;
}

#ifdef RT_SERIAL_USING_DMA
rt_size_t mt2625_dma_transmit(struct rt_serial_device *serial,
                                rt_uint8_t *buf, rt_size_t size, int direction)
{
    RT_ASSERT(serial != RT_NULL);
    struct mt2625_uart *uart = (struct mt2625_uart *)serial->parent.user_data;
    RT_ASSERT(uart != RT_NULL);

    if (direction == RT_SERIAL_DMA_TX)
    {
        return hal_uart_send_dma(uart->uart_port, buf, size);
    }
    else if (direction == RT_SERIAL_DMA_RX)
    {
        rt_kprintf("No implemented!\n");
        return 0;
    }
    else
    {
        rt_kprintf("No implemented!\n");
        return 0;
    }
    
    return 0;
}
#endif

static const struct rt_uart_ops mt2625_uart_ops =
{
    .configure = mt2625_configure,
    .control = mt2625_control,
    .putc = mt2625_putc,
    .getc = mt2625_getc,
#ifdef RT_SERIAL_USING_DMA
    .dma_transmit = mt2625_dma_transmit
#endif
};

int rt_hw_usart_init(void)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    rt_err_t result = 0;
    rt_uint32_t flag;

    for (int i = 0; i < (sizeof(uart_obj_sets)/sizeof(uart_obj_sets[0])); i ++)
    {
        uart_obj_sets[i].serial.ops = &mt2625_uart_ops;;
        uart_obj_sets[i].serial.config = config;

#if defined(RT_SERIAL_USING_DMA)
        if(uart_obj_sets[i].dma.using_rx_dma_mode)
        {
            flag = RT_DEVICE_FLAG_RDWR | 
                    RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX;
        }
        else
#endif
        {
            flag = RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX;
        }

        /* register UART device */
        result = rt_hw_serial_register(&uart_obj_sets[i].serial, uart_obj_sets[i].name,
                                        flag,
                                        &uart_obj_sets[i]);
        RT_ASSERT(result == RT_EOK);
    }

    return result;
}

#else

#warning "Please select one uart device first!"

int rt_hw_usart_init(void)
{
    return 0;
}
#endif /* BSP_USING_UART0 || BSP_USING_UART1 || BSP_USING_UART2 || BSP_USING_UART3 */

#endif /* RT_USING_SERIAL */
