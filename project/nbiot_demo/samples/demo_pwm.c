/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-08-07     MurphyZhao        first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#if defined(RT_USING_PWM) && defined(BSP_USING_PWM)
#define TEST_DEV_NAME   "pwm3"

#define TEST_CNT            (20u)

#define DBG_TAG               "demo.pwm"
#define DBG_LVL               DBG_LOG
#include <rtdbg.h>

static int mt2625_pwm_sample(int argc, char *argv[])
{
    struct rt_device_pwm *dev;

    rt_int32_t period, pulse, dir;
    rt_int16_t cnt = 0;

    period = 500000;    /* period: 0.5ms, unit: ns */
    pulse = 0;          /* PWM pulse, unit: ns */
    dir = 1;

    char dev_name[RT_NAME_MAX];

    if (argc != 2)
    {
        LOG_E("Param error.");
        LOG_I("Please input 'demo_pwm 1' or 'demo_pwm 3'");
        return -RT_ERROR;
    }

    rt_snprintf(dev_name, sizeof(dev_name), "pwm%c", argv[1][0]);
    LOG_I("Test <%s> device", dev_name);

    dev = (struct rt_device_pwm *)rt_device_find(dev_name);
    if (dev == RT_NULL)
    {
        LOG_E("PWM sample run failed! can't find %s device!", dev_name);
        return -RT_ERROR;
    }

    LOG_I("Set 50%% duty cycle");
    pulse = 250000;
    if (RT_EOK != rt_pwm_set(dev, 0, period, pulse)) /* channel is not used in mt2625 */
    {
        return -RT_ERROR;
    }

    if (RT_EOK != rt_pwm_enable(dev, 0))
    {
        return -RT_ERROR;
    }

    rt_thread_mdelay(10000);

    LOG_I("Start dynamic duty cycle");
    pulse = 0;
    while (1)
    {
        if (RT_EOK != rt_pwm_set(dev, 0, period, pulse))
        {
            return -RT_ERROR;
        }

        rt_thread_mdelay(30);

        pulse = dir ? (pulse + 5000) : (pulse - 5000);

        if (pulse >= period)
        {
            pulse = period;
            dir = 0;
        }
        else if (0 >= pulse)
        {
            pulse = 0;
            dir = 1;
            cnt++;
            if (cnt > TEST_CNT)
            {
                break;
            }
        }
    }

    LOG_I("Test <%s> device end", dev_name);
    return RT_EOK;
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(mt2625_pwm_sample, demo_pwm, demo_pwm <pwm device num>);
#endif /* FINSH_USING_MSH */
#endif /* RT_USING_PWM && BSP_USING_PWM */
