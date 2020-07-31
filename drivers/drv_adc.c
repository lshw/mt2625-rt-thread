/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-08-06     MurphyZhao        first version
 */

#include "rtthread.h"

#if defined(BSP_USING_ADC0) || defined(BSP_USING_ADC1) || defined(BSP_USING_ADC2) || defined(BSP_USING_ADC3) || defined(BSP_USING_ADC4)

#ifdef RT_USING_ADC

#include <rtdevice.h>
#include "drv_adc.h"
#include "hal_adc.h"
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_gpio.h"

#define DRV_DEBUG

#ifdef DRV_DEBUG

#define DBG_TAG               "drv.adc"
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_ERROR
#endif /* DRV_DEBUG */
#include <rtdbg.h>

struct mt2625_adc_dev
{
    struct rt_adc_device device;
    const char *name;
    hal_gpio_pin_t pin;
    uint8_t function_index; /* defined in hal_pinmux_define.h */
    hal_adc_channel_t channel;
};

struct mt2625_adc_dev adc_obj_sets[] = 
{
#ifdef BSP_USING_ADC0
    {
        .name = "adc0",
        .pin = HAL_GPIO_30,
        .function_index = HAL_GPIO_30_AUXADC0,
        .channel = HAL_ADC_CHANNEL_0
    },
#endif
#ifdef BSP_USING_ADC1
    {
        .name = "adc1",
        .pin = HAL_GPIO_31,
        .function_index = HAL_GPIO_31_AUXADC1,
        .channel = HAL_ADC_CHANNEL_1
    },
#endif
#ifdef BSP_USING_ADC2
    {
        .name = "adc2",
        .pin = HAL_GPIO_32,
        .function_index = HAL_GPIO_32_AUXADC2,
        .channel = HAL_ADC_CHANNEL_2
    },
#endif
#ifdef BSP_USING_ADC3
    {
        .name = "adc3",
        .pin = HAL_GPIO_33,
        .function_index = HAL_GPIO_33_AUXADC3,
        .channel = HAL_ADC_CHANNEL_3
    },
#endif
#ifdef BSP_USING_ADC4
    {
        .name = "adc4",
        .pin = HAL_GPIO_34,
        .function_index = HAL_GPIO_34_AUXADC4,
        .channel = HAL_ADC_CHANNEL_4
    },
#endif
};

static rt_err_t adc_dev_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    RT_ASSERT(device != RT_NULL);
    struct mt2625_adc_dev *adc = (struct mt2625_adc_dev *)device->parent.user_data;

    if (enabled)
    {
        hal_gpio_init(adc->pin);
        hal_pinmux_set_function(adc->pin, adc->function_index);
        hal_adc_init();
    }
    else
    {
        hal_adc_deinit();//Deinit ADC module when job done.
    }

    return RT_EOK;
}

static rt_err_t adc_dev_value_get(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
    hal_adc_status_t ret;
    RT_ASSERT(device != RT_NULL);
    struct mt2625_adc_dev *adc = (struct mt2625_adc_dev *)device->parent.user_data;

    /* get ADC value */
    
    uint32_t adc_data;

    if (HAL_ADC_STATUS_OK != (ret = hal_adc_get_data_polling(adc->channel, &adc_data)))
    {
        LOG_E("Get adc value failed!");
        *value = 0;
    }
    else
    {
        *value = adc_data;
    }

    return (ret == HAL_ADC_STATUS_OK) ? RT_EOK : -RT_ERROR;
}

static const struct rt_adc_ops adc_dev_ops =
{
    .enabled = adc_dev_enabled,
    .convert = adc_dev_value_get,
};

int drv_adc_init(void)
{
    rt_err_t result = RT_EOK;

    for (int i = 0; i < (sizeof(adc_obj_sets)/(sizeof(adc_obj_sets[0]))); i++)
    {
        /* register ADC device */
        if (rt_hw_adc_register(&adc_obj_sets[i].device, adc_obj_sets[i].name, &adc_dev_ops, &adc_obj_sets[i]) != RT_EOK)
        {
            LOG_E("Register <%s> failed", adc_obj_sets[i].name);
            result = -RT_ERROR;
        }
    }

    return result;
}

#ifdef RT_USING_COMPONENTS_INIT
INIT_DEVICE_EXPORT(drv_adc_init);
#endif
#endif /* RT_USING_ADC */
#endif /* BSP_USING_ADC */
