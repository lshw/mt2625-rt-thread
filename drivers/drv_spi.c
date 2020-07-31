/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-08-07     MurphyZhao        first version
 */

#include "rtthread.h"

#if defined(BSP_USING_SPI0) || defined(BSP_USING_SPI1)

#ifdef RT_USING_SPI

#include <rtdevice.h>
#include "drv_spi.h"
#include "hal_spi_master.h"
#include "hal_spi_master_internal.h"
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_gpio.h"

// #define DRV_DEBUG

#define DBG_TAG               "drv.spi"
#ifdef DRV_DEBUG
#define DBG_LVL               DBG_LOG /* DBG_INFO */
#else
#define DBG_LVL               DBG_ERROR
#endif /* DRV_DEBUG */
#include <rtdbg.h>

/**
 * Using SPI master polling mode.
 * Using SPI master DMA mode.
 * Using SPI master DMA blocking mode.
 * 
 * SPI0
 * HAL_GPIO_8  -> HAL_GPIO_8_SPI_MST0_CS
 * HAL_GPIO_9  -> HAL_GPIO_9_SPI_MST0_MISO
 * HAL_GPIO_10 -> HAL_GPIO_10_SPI_MST0_MOSI
 * HAL_GPIO_11 -> HAL_GPIO_11_SPI_MST0_SCK
 * 
 * SPI1
 * HAL_GPIO_19  -> HAL_GPIO_19_SPI_MST1_CS
 * HAL_GPIO_20  -> HAL_GPIO_20_SPI_MST1_MISO
 * HAL_GPIO_21 -> HAL_GPIO_21_SPI_MST1_MOSI
 * HAL_GPIO_22 -> HAL_GPIO_22_SPI_MST1_SCK
*/

struct mt2625_spi_pinmux
{
    hal_gpio_pin_t pin_cs;
    rt_uint8_t pin_cs_func;

    hal_gpio_pin_t pin_mosi;
    rt_uint8_t pin_mosi_func;

    hal_gpio_pin_t pin_miso;
    rt_uint8_t pin_miso_func;

    hal_gpio_pin_t pin_sck;
    rt_uint8_t pin_sck_func;
};

enum mt2625_work_mode
{
    MASTER_POLLING_MODE = 0,
    MASTER_DMA_MODE,
    MASTER_DMA_BLOCKING_MODE
};

#define SPI_USING_DMA_FLAG         (1<<0)
#define SPI_USING_DMA_BLOCK_FLAG   (1<<1)

struct mt2625_spi_bus
{
    struct rt_spi_bus spi_bus;
    struct mt2625_spi_pinmux pinmux;
    hal_spi_master_config_t spi_config;
    hal_spi_master_port_t master_port;
    struct rt_spi_configuration *rt_spi_cfg;
    uint8_t work_mode; /* 0:polling mode; 1:dma mode; 2:dma block mode */
    const char *name;  /* spi bus name */
};

static struct mt2625_spi_bus spi_obj_sets[] =
{
#ifdef BSP_USING_SPI0
    {
        .name = "spi0",
        .pinmux = {
            .pin_cs        = HAL_GPIO_8,
            .pin_cs_func   = HAL_GPIO_8_SPI_MST0_CS,
            .pin_mosi      = HAL_GPIO_10,
            .pin_mosi_func = HAL_GPIO_10_SPI_MST0_MOSI,
            .pin_miso      = HAL_GPIO_9,
            .pin_miso_func = HAL_GPIO_9_SPI_MST0_MISO,
            .pin_sck       = HAL_GPIO_11,
            .pin_sck_func  = HAL_GPIO_11_SPI_MST0_SCK
        },
        .master_port = HAL_SPI_MASTER_0,
#ifdef BSP_USING_SPI0_POLLING_MODE
        .work_mode = MASTER_POLLING_MODE
#else

#ifdef BSP_USING_SPI0_DMA_MODE
        .work_mode = MASTER_DMA_MODE
#else defined(BSP_USING_SPI0_DMA_BLOCK_MODE)
        .work_mode = MASTER_DMA_BLOCKING_MODE
#endif /* BSP_USING_SPI0_DMA_MODE */
#endif /* BSP_USING_SPI0_POLLING_MODE */
    },
#endif /* BSP_USING_SPI0 */

#ifdef BSP_USING_SPI1
    {
        .name = "spi1",
        .pinmux = {
            .pin_cs        = HAL_GPIO_19,
            .pin_cs_func   = HAL_GPIO_19_SPI_MST1_CS,
            .pin_mosi      = HAL_GPIO_21,
            .pin_mosi_func = HAL_GPIO_21_SPI_MST1_MOSI,
            .pin_miso      = HAL_GPIO_20,
            .pin_miso_func = HAL_GPIO_20_SPI_MST1_MISO,
            .pin_sck       = HAL_GPIO_22,
            .pin_sck_func  = HAL_GPIO_22_SPI_MST1_SCK
        },
        .master_port = HAL_SPI_MASTER_1,
#ifdef BSP_USING_SPI1_POLLING_MODE
        .work_mode = MASTER_POLLING_MODE
#else

#ifdef BSP_USING_SPI1_DMA_MODE
        .work_mode = MASTER_DMA_MODE
#else defined(BSP_USING_SPI1_DMA_BLOCK_MODE)
        .work_mode = MASTER_DMA_BLOCKING_MODE
#endif /* BSP_USING_SPI1_DMA_MODE */
#endif /* BSP_USING_SPI1_POLLING_MODE */
    },
#endif /* BSP_USING_SPI1 */
};

static rt_err_t _spi_init(struct mt2625_spi_bus *spi_drv, struct rt_spi_configuration *cfg)
{
    RT_ASSERT(spi_drv != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    spi_drv->spi_config.slave_port = HAL_SPI_MASTER_SLAVE_0;

    if (cfg->mode & RT_SPI_MSB)
    {
        spi_drv->spi_config.bit_order = HAL_SPI_MASTER_MSB_FIRST;
    }
    else
    {
        spi_drv->spi_config.bit_order = HAL_SPI_MASTER_LSB_FIRST;
    }

    /* spi frequency must be 104/N, N is a multiple of 2 */
    if (cfg->max_hz >= 52000000)
    {
        cfg->max_hz = 52000000;
    }
    else if (cfg->max_hz >= 26000000)
    {
        cfg->max_hz = 26000000;
    }
    else if (cfg->max_hz >= 13000000)
    {
        cfg->max_hz = 13000000;
    }
    else
    {
        cfg->max_hz = 13000000;
    }

    spi_drv->spi_config.clock_frequency = cfg->max_hz;

    if (cfg->mode & RT_SPI_CPHA)
    {
        spi_drv->spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE1;
    }
    else
    {
        spi_drv->spi_config.phase = HAL_SPI_MASTER_CLOCK_PHASE0;
    }

    if (cfg->mode & RT_SPI_CPOL)
    {
        spi_drv->spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY1;
    }
    else
    {
        spi_drv->spi_config.polarity = HAL_SPI_MASTER_CLOCK_POLARITY0;
    }

    if (cfg->mode & RT_SPI_3WIRE)
    {
        LOG_W("Not support RT_SPI_3WIRE mode");
    }

    /* GPIO INIT */
    {
        // Initialize the GPIO, set GPIO pinmux (if EPT tool hasn't been used to configure the related pinmux).
        hal_gpio_init(spi_drv->pinmux.pin_cs);
        hal_gpio_init(spi_drv->pinmux.pin_miso);
        hal_gpio_init(spi_drv->pinmux.pin_mosi);
        hal_gpio_init(spi_drv->pinmux.pin_sck);

        hal_pinmux_set_function(spi_drv->pinmux.pin_cs,   spi_drv->pinmux.pin_cs_func);   // Set the pin to be used as CS signal of SPI.
        hal_pinmux_set_function(spi_drv->pinmux.pin_sck,  spi_drv->pinmux.pin_sck_func);  // Set the pin to be used as SCK signal of SPI.
        hal_pinmux_set_function(spi_drv->pinmux.pin_mosi, spi_drv->pinmux.pin_mosi_func); // Set the pin to be used as MOSI signal of SPI.
        hal_pinmux_set_function(spi_drv->pinmux.pin_miso, spi_drv->pinmux.pin_miso_func); // Set the pin to be used as MISO signal of SPI.
    }

    if (cfg->mode & RT_SPI_SLAVE)
    {
        LOG_W("SPI slave mode is not supported");
        return -RT_ERROR;
    }
    else
    {
        LOG_D("SPI master mode");
        // Initialize the SPI master.
        if (HAL_SPI_MASTER_STATUS_OK != 
            hal_spi_master_init(spi_drv->master_port, &spi_drv->spi_config))
        {
            LOG_E("<%s> master init failed!", spi_drv->name);
            return -RT_ERROR;
        }
    }

    LOG_D("<%s> init done", spi_drv->name);
    return RT_EOK;
}

static rt_err_t mt2625_spi_dma_send(struct mt2625_spi_bus *spi_drv, struct rt_spi_message *message)
{
    hal_spi_master_status_t status = HAL_SPI_MASTER_STATUS_OK;
    if (message->length <= 0)
    {
        LOG_E("message len error");
        return -RT_ERROR;
    }

    if (HAL_SPI_MASTER_STATUS_OK != 
        (status = hal_spi_master_send_dma(
            spi_drv->master_port, message->send_buf, message->length)))
    {
        LOG_E("<%s> send failed in dma mode, error:%d; send buf:%p; len:%d", 
            spi_drv->name, status, message->send_buf, message->length);
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t mt2625_spi_dma_block_send(struct mt2625_spi_bus *spi_drv, struct rt_spi_message *message)
{
    hal_spi_master_status_t status = HAL_SPI_MASTER_STATUS_OK;
    if (message->length <= 0)
    {
        LOG_E("message len error");
        return -RT_ERROR;
    }

    if (HAL_SPI_MASTER_STATUS_OK != 
        (status = hal_spi_master_send_dma_blocking(
            spi_drv->master_port, message->send_buf, message->length)))
    {
        LOG_E("<%s> send failed in dma block mode, error:%d", spi_drv->name, status);
        return -RT_ERROR;
    }
    return RT_EOK;
}

static rt_err_t mt2625_spi_polling_send(struct mt2625_spi_bus *spi_drv, struct rt_spi_message *message)
{
    rt_err_t state = RT_EOK;
    rt_size_t message_length, already_send_length;
    hal_spi_master_send_and_receive_config_t send_recv_cfg;

    rt_uint16_t send_length;
    rt_uint8_t *recv_buf;
    uint8_t *send_buf;

    message_length = message->length;
    recv_buf = message->recv_buf;
    send_buf = message->send_buf;

    if (message_length <= 0)
    {
        LOG_E("message len error");
        return -RT_ERROR;
    }

    while (message_length)
    {
        /* the HAL library use uint16 to save the data length */
        if (message_length > HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE)
        {
            send_length = HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
            message_length = message_length - HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
        }
        else
        {
            send_length = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_send_length = message->length - send_length - message_length;
        send_buf = (rt_uint8_t *)message->send_buf + already_send_length;
        recv_buf = (rt_uint8_t *)message->recv_buf + already_send_length;

        if (HAL_SPI_MASTER_STATUS_OK != 
            hal_spi_master_send_polling(
                spi_drv->master_port, send_buf, send_length))
        {
            LOG_E("<%s> send failed in polling mode", spi_drv->name);
            state = -RT_ERROR;
            goto __exit;
        }
    }

__exit:
    return state;
}

static rt_err_t mt2625_spi_dma_recv(struct mt2625_spi_bus *spi_drv, struct rt_spi_message *message)
{
    return RT_EOK;
}

static rt_err_t mt2625_spi_dma_block_recv(struct mt2625_spi_bus *spi_drv, struct rt_spi_message *message)
{
    return RT_EOK;
}

static rt_err_t mt2625_spi_polling_recv(struct mt2625_spi_bus *spi_drv, struct rt_spi_message *message)
{
    rt_err_t state = RT_EOK;
    rt_size_t message_length, already_recv_length;
    hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;

    rt_uint16_t recv_length;
    rt_uint8_t *recv_buf;

    message_length = message->length;

    while (message_length)
    {
        /* the HAL library use uint16 to save the data length */
        if (message_length > HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE)
        {
            recv_length = HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
            message_length = message_length - HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
        }
        else
        {
            recv_length = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_recv_length = message->length - recv_length - message_length;
        recv_buf = (rt_uint8_t *)message->recv_buf + already_recv_length;

        spi_send_and_receive_config.send_data = recv_buf;
        spi_send_and_receive_config.send_length = 1;
        spi_send_and_receive_config.receive_buffer = recv_buf;
        spi_send_and_receive_config.receive_length = recv_length + 1;

        LOG_D("<%s> recv:%p-%d", recv_buf, recv_length);
        if (HAL_SPI_MASTER_STATUS_OK != 
            hal_spi_master_send_and_receive_polling(
                spi_drv->master_port, &spi_send_and_receive_config))
        {
            LOG_E("<%s> spi master send adn recv data failed", spi_drv->name);
            state = -RT_ERROR;
            goto __exit;
        }
    }
__exit:
    return state;
}

static rt_err_t mt2625_spi_polling_send_recv(struct mt2625_spi_bus *spi_drv, struct rt_spi_message *message)
{
    rt_err_t state = RT_EOK;
    rt_size_t message_length, already_recv_length;
    hal_spi_master_send_and_receive_config_t spi_send_and_receive_config;

    rt_uint16_t recv_length;
    rt_uint8_t *send_buf;
    rt_uint8_t *recv_buf;

    message_length = message->length;

    while (message_length)
    {
        /* the HAL library use uint16 to save the data length */
        if (message_length > HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE)
        {
            recv_length = HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
            message_length = message_length - HAL_SPI_MAXIMUM_POLLING_TRANSACTION_SIZE;
        }
        else
        {
            recv_length = message_length;
            message_length = 0;
        }

        /* calculate the start address */
        already_recv_length = message->length - recv_length - message_length;
        send_buf = (rt_uint8_t *)message->send_buf + already_recv_length;;
        recv_buf = (rt_uint8_t *)message->recv_buf + already_recv_length;

        spi_send_and_receive_config.send_data = send_buf;
        spi_send_and_receive_config.send_length = recv_length;
        spi_send_and_receive_config.receive_buffer = recv_buf;
        spi_send_and_receive_config.receive_length = recv_length + 1;

        LOG_D("<%s> recv:%p-%d", recv_buf, recv_length);
        hal_spi_master_send_and_receive_polling(spi_drv->master_port, &spi_send_and_receive_config);
    }
__exit:
    return state;
}

static rt_uint32_t spixfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    rt_err_t state = RT_EOK;
    struct mt2625_spi_bus *spi_drv;

    rt_uint8_t flag = 0x00;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(device->parent.user_data != RT_NULL);
    RT_ASSERT(message != RT_NULL);

    spi_drv = rt_container_of(device->bus, struct mt2625_spi_bus, spi_bus);

    struct mt2625_spi_cs *cs = device->parent.user_data;

    if (message->cs_take)
    {
        hal_gpio_set_output(cs->cs_pin, HAL_GPIO_DATA_LOW);
    }

    LOG_D("<%s> transfer prepare and start", spi_drv->name);
    LOG_D("<%s> sendbuf: %p, recvbuf: %p, length: %d",
            spi_drv->name, message->send_buf, message->recv_buf, message->length);

    if (message->send_buf) flag |= (1 << 0);
    if (message->recv_buf) flag |= (1 << 1);

    switch (flag)
    {
        case 0x00: /* no send, no recv */
            LOG_D("No send, no recv");
            break;
        case 0x01: /* send, no recv  */
            LOG_D("send, no recv");
            /* dma mode */
            if (spi_drv->work_mode & SPI_USING_DMA_FLAG)
            {
                state = mt2625_spi_dma_send(spi_drv, message);
            }
            /* dma block mode */
            else if (spi_drv->work_mode & SPI_USING_DMA_BLOCK_FLAG)
            {
                state = mt2625_spi_dma_block_send(spi_drv, message);
            }
            /* polling mode */
            else
            {
                state = mt2625_spi_polling_send(spi_drv, message);
            }
            break;
        case 0x10: /* no send, recv */
            LOG_D("No send, recv");
            if (spi_drv->work_mode & SPI_USING_DMA_FLAG) /* dma mode */
            {
                LOG_D("dma recv mode");
            }
            else if (spi_drv->work_mode & SPI_USING_DMA_BLOCK_FLAG) /* dma block mode */
            {
                LOG_D("dma block recv mode");
            }
            else /* polling mode */
            {
                LOG_D("polling recv mode");
                state = mt2625_spi_polling_recv(spi_drv, message);
            }
            break;
        case 0x11: /* send && recv; not implement */
            LOG_D("send and recv");
            mt2625_spi_polling_send_recv(spi_drv, message);
            break;
        default: /* error */
            state = -RT_ERROR;
            break;
    }

    if (state != RT_EOK)
    {
        LOG_I("spi transfer error : %d", state);
        message->length = 0;
    }
    else
    {
        LOG_D("%s transfer done", spi_drv->name);
    }

    if (message->cs_release)
    {
        hal_gpio_set_output(cs->cs_pin, HAL_GPIO_DATA_HIGH);
    }

    return message->length;
}

static rt_err_t spi_configure(struct rt_spi_device *device,
                              struct rt_spi_configuration *configuration)
{
    struct mt2625_spi_bus *spi_drv;
    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(configuration != RT_NULL);

    spi_drv =  rt_container_of(device->bus, struct mt2625_spi_bus, spi_bus);
    RT_ASSERT(spi_drv != RT_NULL);

    spi_drv->rt_spi_cfg = configuration;

    return _spi_init(spi_drv, configuration);
}

static const struct rt_spi_ops stm_spi_ops =
{
    .configure = spi_configure,
    .xfer = spixfer,
};


/**
  * Attach the spi device to SPI bus, this function must be used after initialization.
  */
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, uint16_t cs_pin, uint16_t cs_pin_func)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;
    struct mt2625_spi_cs *cs_pin_ops;

    hal_gpio_init(cs_pin);
    hal_pinmux_set_function(cs_pin, cs_pin_func);
    hal_gpio_set_output(cs_pin, HAL_GPIO_DATA_LOW);

    /* attach the device to spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);

    cs_pin_ops = (struct mt2625_spi_cs *)rt_malloc(sizeof(struct mt2625_spi_cs));
    RT_ASSERT(cs_pin_ops != RT_NULL);

    cs_pin_ops->cs_pin = cs_pin;
    cs_pin_ops->cs_pin_func = cs_pin_func;

    result = rt_spi_bus_attach_device(spi_device, device_name, bus_name, (void *)cs_pin_ops);
    if (result != RT_EOK)
    {
        LOG_E("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }

    RT_ASSERT(result == RT_EOK);

    LOG_D("%s attach to %s done", device_name, bus_name);

    return result;
}

int drv_spi_init(void)
{
    rt_err_t result = RT_EOK;
    for (int i = 0; i < (sizeof(spi_obj_sets) / sizeof(spi_obj_sets[0])); i++)
    {
        result = rt_spi_bus_register(&spi_obj_sets[i].spi_bus, spi_obj_sets[i].name, &stm_spi_ops);
        RT_ASSERT(result == RT_EOK);

        LOG_D("%s bus init done", spi_obj_sets[i].name);
    }
    return result;
}
#ifdef RT_USING_COMPONENTS_INIT
INIT_BOARD_EXPORT(drv_spi_init);
#endif
#endif /* RT_USING_SPI */
#endif /* BSP_USING_SPI0 || BSP_USING_SPI1 */
