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

#if defined(BSP_USING_PWM0) || defined(BSP_USING_PWM1) || defined(BSP_USING_PWM2) || defined(BSP_USING_PWM3)

#ifdef RT_USING_PWM

#include <rtdevice.h>
#include "drv_pwm.h"
#include "hal_pwm.h"
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_gpio.h"

#define DRV_DEBUG

#ifdef DRV_DEBUG

#define DBG_TAG               "drv.pwm"
#define DBG_LVL               DBG_INFO
#else
#define DBG_LVL               DBG_ERROR
#endif /* DRV_DEBUG */
#include <rtdbg.h>

struct mt2625_pwm_dev
{
    struct rt_device_pwm device;
    hal_gpio_pin_t pin;
    rt_uint8_t function_index; /* defined in hal_pinmux_define.h */
    hal_pwm_channel_t channel;
    hal_pwm_source_clock_t clock;
    uint32_t pwm_cnt;
    uint8_t is_inited;
    const char *name;
};

/**
 * HAL_GPIO_4_PWM0 --> uart2_tx | HAL_GPIO_1_PWM0 --> NC(M5311) | HAL_GPIO_0_PWM0 --> NC(M5311)
 * HAL_GPIO_8_PWM1 | HAL_GPIO_7_PWM1
 * HAL_GPIO_13_PWM2 --> NC(M5311) | HAL_GPIO_15_PWM2 --> NC(M5311)
 * HAL_GPIO_26_PWM3 --> NET STATUS LED
*/
static struct mt2625_pwm_dev pwm_obj_sets[] =
{
#ifdef BSP_USING_PWM0
    {
        .name = "pwm0",
        .pin  = HAL_GPIO_4,
        .function_index = HAL_GPIO_4_PWM0,
        .channel = HAL_PWM_0,
#ifdef BSP_USING_PWM0_CLOCK
        .clock = BSP_USING_PWM0_CLOCK,
#else
        .clock = HAL_PWM_CLOCK_32KHZ,
#endif
        .pwm_cnt = 0,
        .is_inited = 0
    },
#endif

#ifdef BSP_USING_PWM1
    {
        .name = "pwm1",
        .pin  = HAL_GPIO_7,
        .function_index = HAL_GPIO_7_PWM1,
        .channel = HAL_PWM_1,
#ifdef BSP_USING_PWM1_CLOCK
        .clock = BSP_USING_PWM1_CLOCK,
#else
        .clock = HAL_PWM_CLOCK_32KHZ,
#endif
        .pwm_cnt = 0,
        .is_inited = 0
    },
#endif

#ifdef BSP_USING_PWM2
    {
        .name = "pwm2",
        .pin  = HAL_GPIO_13,
        .function_index = HAL_GPIO_13_PWM2,
        .channel = HAL_PWM_2,
#ifdef BSP_USING_PWM2_CLOCK
        .clock = BSP_USING_PWM2_CLOCK,
#else
        .clock = HAL_PWM_CLOCK_32KHZ,
#endif
        .pwm_cnt = 0,
        .is_inited = 0
    },
#endif

#ifdef BSP_USING_PWM3
    {
        .name = "pwm3",
        .pin  = HAL_GPIO_26,
        .function_index = HAL_GPIO_26_PWM3,
        .channel = HAL_PWM_3,
#ifdef BSP_USING_PWM0_CLOCK
        .clock = BSP_USING_PWM3_CLOCK,
#else
        .clock = HAL_PWM_CLOCK_32KHZ,
#endif
        .pwm_cnt = 0,
        .is_inited = 0
    },
#endif
};

static rt_err_t drv_pwm_set(struct mt2625_pwm_dev *dev, struct rt_pwm_configuration *configuration)
{
    uint32_t frequency;
    uint32_t duty_ratio;
    uint32_t total_count;
    uint32_t duty_cycle;

    if (dev->is_inited == 0)
    {
        hal_gpio_deinit(dev->pin);
        hal_gpio_init(dev->pin);
        hal_gpio_set_direction(HAL_GPIO_26, HAL_GPIO_DIRECTION_OUTPUT);
        hal_pinmux_set_function(dev->pin, dev->function_index);
        if(HAL_PWM_STATUS_OK != hal_pwm_init(dev->channel, dev->clock))
        {
            LOG_E("<%s> init failed!", dev->name);
            return -RT_ERROR;
        }
        LOG_D("PWM channel:%d; clock:%d", dev->channel, dev->clock);
        dev->is_inited = 1;
    }

    /* Convert nanosecond to frequency and duty cycle. 1s = 1 * 1000 * 1000 * 1000 ns */
    frequency = 1000000000UL/configuration->period;

    if(HAL_PWM_STATUS_OK != hal_pwm_set_frequency(dev->channel, frequency, &total_count))
    {
        LOG_E("<%s> set frequency <%ld> failed!", dev->name, frequency);
        return -RT_ERROR;
    }
    LOG_D("PWM frequency:%d; total cnt:%d", frequency, total_count);

    dev->pwm_cnt = total_count;

    /* duty_cycle is calcauted as a product of application's duty_ratio and hardware's total_count. */
    duty_ratio = (configuration->pulse * 100)/configuration->period;
    duty_cycle = (total_count * duty_ratio)/100;

    /* Enable PWM to start the timer. */
    if(HAL_PWM_STATUS_OK != hal_pwm_set_duty_cycle(dev->channel, duty_cycle))
    {
        LOG_E("<%s> set duty cycle failed!", dev->name);
        return -RT_ERROR;
    }
    LOG_D("PWM duty cycle:%d; duty_ratio:%d", duty_cycle, duty_ratio);

    return RT_EOK;
}

static rt_err_t drv_pwm_control(struct rt_device_pwm *device, int cmd, void *arg)
{
    uint32_t frequency;
    uint32_t duty_ratio;
    uint32_t duty_cycle;
    struct rt_pwm_configuration *configuration;
    struct mt2625_pwm_dev *dev;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(arg != RT_NULL);

    configuration = (struct rt_pwm_configuration *)arg;
    dev = (struct mt2625_pwm_dev *)device->parent.user_data;
    RT_ASSERT(dev != RT_NULL);

    switch (cmd)
    {
    case PWM_CMD_ENABLE:

        if (dev->is_inited == 0)
        {
            LOG_E("Please first call rt_pwm_set function!");
            return -RT_ERROR;
        }

        if (HAL_PWM_STATUS_OK != hal_pwm_start(dev->channel))
        {
            LOG_E("<%s> start failed!", dev->name);
            return -RT_ERROR;
        }
        break;

    case PWM_CMD_DISABLE:
        hal_pwm_stop(dev->channel);
        hal_pwm_deinit(dev->channel);
        dev->is_inited = 0;
        break;

    case PWM_CMD_SET:
        if (RT_EOK != drv_pwm_set(dev, configuration))
        {
            return -RT_ERROR;
        }
        break;

    case PWM_CMD_GET:

        if (HAL_PWM_STATUS_OK != hal_pwm_get_frequency(dev->channel, &frequency))
        {
            LOG_E("<%s> get frequency failed!", dev->name);
            return -RT_ERROR;
        }
        if (HAL_PWM_STATUS_OK != hal_pwm_get_duty_cycle(dev->channel, &duty_cycle))
        {
            LOG_E("<%s> get duty cycle failed!", dev->name);
            return -RT_ERROR;
        }

        configuration->period = 1000000000UL/frequency; // ns
        duty_ratio = duty_cycle * 100 / dev->pwm_cnt;
        configuration->pulse = duty_ratio * configuration->period / 100;
        break;

    default:
        return -RT_EINVAL;
    }
    return RT_EOK;
}

static struct rt_pwm_ops drv_ops =
{
    drv_pwm_control
};

int drv_pwm_init(void)
{
    int i = 0;
    int result = RT_EOK;

    for (i = 0; i < sizeof(pwm_obj_sets) / sizeof(pwm_obj_sets[0]); i++)
    {
        /* register pwm device */
        if (rt_device_pwm_register(&pwm_obj_sets[i].device, pwm_obj_sets[i].name, &drv_ops, &pwm_obj_sets[i]) != RT_EOK)
        {
            LOG_E("Register <%s> device failed", pwm_obj_sets[i].name);
            result = -RT_ERROR;
        }
    }

    return result;
}
#ifdef RT_USING_COMPONENTS_INIT
INIT_DEVICE_EXPORT(drv_pwm_init);
#endif
#endif /* RT_USING_PWM */
#endif /* BSP_USING_PWM0 || BSP_USING_PWM1 || BSP_USING_PWM2 || BSP_USING_PWM3 */
