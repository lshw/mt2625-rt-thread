/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-13     MurphyZhao        first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#ifdef RT_USING_WDT

#include "drv_wdt.h"

#define IWDG_DEVICE_NAME    "wdt"

#define DBG_TAG               "demo.wdt"
#define DBG_LVL               DBG_LOG
#include <rtdbg.h>

static rt_device_t wdt_dev;

static void idle_hook(void)
{
    /* feed wdt in rt-thread idle thread  */
    rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}

/**
 * demo_wdt -t 2000 -f 0
 * -t set wdt timeout
 * -f 0: not feed wdt; 1: feed wdt in idle hook
*/
static int mtk_wdt_sample(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;
    rt_uint8_t feed_flg = 0;
    static rt_uint8_t is_inited = 0;
    rt_uint32_t timeout = 2000;    /* timeout (unit: ms) */
    char device_name[RT_NAME_MAX];

    if (argc != 5)
    {
        rt_kprintf("\nUsage:\ndemo_wdt -t <timeout> -f <0/1>\n");
        rt_kprintf("-t set wdt timeout(ms)\n");
        rt_kprintf("-f 0: not feed wdt; 1: feed wdt in idle hook\n");
        rt_kprintf("eg: demo_wdt -t 2000 -f 0\n\n");
        return -RT_ERROR;
    }

    timeout = atoi(argv[2]);
    feed_flg = atoi(argv[4]);

    if (is_inited == 0)
    {
        rt_strncpy(device_name, IWDG_DEVICE_NAME, RT_NAME_MAX);

        wdt_dev = rt_device_find(device_name);
        if (!wdt_dev)
        {
            LOG_E("find <%s> failed!", device_name);
            return -RT_ERROR;
        }

        // remove `vApplicationIdleHook`
        extern void vApplicationIdleHook( void );
        rt_thread_idle_delhook(vApplicationIdleHook);

        /* init wdt device */
        ret = rt_device_init(wdt_dev);
        if (ret != RT_EOK)
        {
            LOG_E("initialize <%s> failed!", device_name);
            return -RT_ERROR;
        }
        is_inited = 1;
    }

    ret = rt_device_control(wdt_dev, RT_DEVICE_CTRL_WDT_SET_TIMEOUT, &timeout);
    if (ret != RT_EOK)
    {
        LOG_E("set <%s> timeout failed!", device_name);
        return -RT_ERROR;
    }

    if (feed_flg)
    {
        rt_thread_idle_sethook(idle_hook);
    }
    else
    {
        rt_thread_idle_delhook(idle_hook);
        LOG_I("wdt will reset system after <%d> millisecond.", timeout);
        rt_thread_mdelay(timeout * 2);
        LOG_E("must be reset before this log!");
    }

    return ret;
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(mtk_wdt_sample, demo_wdt, watchdog device test);
#endif /* FINSH_USING_MSH */

#endif /* RT_USING_WDT */
