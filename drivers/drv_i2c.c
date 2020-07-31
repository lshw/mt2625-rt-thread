/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-24     MurphyZhao        first version
 */

#include "rtthread.h"
#include "hal_feature_config.h"

#if defined(BSP_USING_I2C) && defined(RT_USING_I2C) && defined(HAL_I2C_MASTER_MODULE_ENABLED)

#include "drv_i2c.h"
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_gpio.h"
#include "hal_i2c_master.h"

#define DBG_TAG                        "drv.i2c"
#define DBG_LVL                        DBG_ERROR
#include <rtdbg.h>

struct mt2625_i2c_bus
{
    struct rt_i2c_bus_device i2c_dev;
    hal_i2c_port_t i2c_port;
    hal_i2c_config_t i2c_config;
    char *name;
};

static rt_size_t i2c_mst_xfer(struct rt_i2c_bus_device *bus,
                                struct rt_i2c_msg msgs[],
                                rt_uint32_t num)
{
    RT_ASSERT(bus != RT_NULL);
    uint32_t i;
    struct mt2625_i2c_bus *device = (struct mt2625_i2c_bus *)bus;

    LOG_D("i2c master transfer...");

    for (i = 0; i < num; i++)
    {
        if (msgs[i].flags == RT_I2C_RD)
        {
            if (HAL_I2C_STATUS_OK != 
                hal_i2c_master_receive_polling(device->i2c_port, msgs[i].addr, msgs[i].buf, msgs[i].len))
            {
                LOG_E("i2c recv failed.");
                return i;
            }
        }
        else if (msgs[i].flags == RT_I2C_WR)
        {
            if (HAL_I2C_STATUS_OK != 
                hal_i2c_master_send_polling(device->i2c_port, msgs[i].addr, msgs[i].buf, msgs[i].len))
            {
                LOG_E("i2c send failed.");
                return i;
            }
        }
    }
    return i;
}

#ifdef BSP_USING_I2C0

static const struct rt_i2c_bus_device_ops i2c0_dev_ops =
{
    .master_xfer = i2c_mst_xfer,
    .slave_xfer = RT_NULL,
    .i2c_bus_control = RT_NULL
};

static struct mt2625_i2c_bus i2c0_dev = 
{
    .name = "i2c0",
    .i2c_port = HAL_I2C_MASTER_0,
    .i2c_config.frequency = HAL_I2C_FREQUENCY_400K
};

static void mt2625_i2c0_dev_init(void)
{
    hal_i2c_status_t status;
    hal_gpio_init(HAL_GPIO_6); // SCL
    hal_gpio_init(HAL_GPIO_7); // SDA

    hal_pinmux_set_function(HAL_GPIO_6, HAL_GPIO_6_I2C0_SCL);
    hal_pinmux_set_function(HAL_GPIO_7, HAL_GPIO_7_I2C0_SDA);

    if(HAL_I2C_STATUS_OK != (status = hal_i2c_master_init(HAL_I2C_MASTER_0, &i2c0_dev.i2c_config)))
    {
        LOG_E("hal_i2c_master_init failed! error: %d", status);
        return;
    }

#ifdef BSP_I2C0_FREQUENCY
    if (BSP_I2C0_FREQUENCY == 50)
    {
        i2c0_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_50K;
    }
    else if (BSP_I2C0_FREQUENCY == 100)
    {
        i2c0_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_100K;
    }
    else if (BSP_I2C0_FREQUENCY == 200)
    {
        i2c0_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_200K;
    }
    else if (BSP_I2C0_FREQUENCY == 300)
    {
        i2c0_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_300K;
    }
    else if (BSP_I2C0_FREQUENCY == 400)
    {
        i2c0_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_400K;
    }
    else if (BSP_I2C0_FREQUENCY == 1000)
    {
        i2c0_dev.i2c_config.frequency = HAL_I2C_FREQUENCY_1M;
    }
#endif /* BSP_I2C0_FREQUENCY */

    i2c0_dev.i2c_dev.ops = &i2c0_dev_ops;

    if (RT_EOK != rt_i2c_bus_device_register(&(i2c0_dev.i2c_dev), i2c0_dev.name))
    {
        LOG_E("i2c device register failed.");
    }
}
#endif /* BSP_USING_I2C0 */

int rt_hw_i2c_init(void)
{
    LOG_D("rt_hw_i2c_init");
#ifdef BSP_USING_I2C0
    mt2625_i2c0_dev_init();
#endif

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_i2c_init);

#endif /* BSP_USING_I2C && RT_USING_I2C && HAL_I2C_MASTER_MODULE_ENABLED */
