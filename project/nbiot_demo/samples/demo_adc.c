/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-08-06     MurphyZhao        first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#if defined(RT_USING_ADC) && defined(BSP_USING_ADC)

#define TEST_CNT            (50u)

#define DBG_TAG               "demo.adc"
#define DBG_LVL               DBG_LOG
#include <rtdbg.h>

/**
 * cmd:
 * test_adc 0
 * or
 * test_adc 1
*/
static int mt2625_adc_sample(int argc, char *argv[])
{
    rt_adc_device_t adc_dev;
    rt_uint32_t value;
    rt_err_t ret = RT_EOK;

    char dev_name[RT_NAME_MAX];

    if (argc != 2)
    {
        LOG_E("Param error.");
        LOG_I("Please input 'demo_adc 0' or 'demo_adc 1'");
        return -RT_ERROR;
    }

    rt_snprintf(dev_name, sizeof(dev_name), "adc%c", argv[1][0]);
    LOG_I("Test <%s> device", dev_name);

    adc_dev = (rt_adc_device_t)rt_device_find(dev_name);
    if (adc_dev == RT_NULL)
    {
        LOG_E("adc sample run failed! can't find %s device!", dev_name);
        return -RT_ERROR;
    }

    ret = rt_adc_enable(adc_dev, 0);
    if (ret != RT_EOK)
    {
        LOG_E("Enable adc device failed!");
        return -RT_ERROR;
    }

    for (int i = 0; i < TEST_CNT; i++)
    {
        value = rt_adc_read(adc_dev, 0);
        LOG_I("the value is :%d", value);
        rt_thread_mdelay(600);
    }

    ret = rt_adc_disable(adc_dev, 0);
    if (ret != RT_EOK)
    {
        LOG_E("Disable adc device failed!");
        return -RT_ERROR;
    }

    LOG_I("Test <%s> device end", dev_name);
    return ret;
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(mt2625_adc_sample, demo_adc, demo_adc <adc num>);
#endif /* FINSH_USING_MSH */

#endif /* RT_USING_ADC && BSP_USING_ADC */
