/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-13     MurphyZhao        first version
 */

#include "rtthread.h"

#ifdef RT_USING_WDT

#include <rtdevice.h>
#include "drv_wdt.h"
#include "hal_wdt.h"
#include "hal_wdt_internal.h"
#include "system.h"
#include "hal_platform.h"

#define DRV_DEBUG

#ifdef DRV_DEBUG

#define DBG_TAG               "drv.wdt"
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif /* DRV_DEBUG */
#include <rtdbg.h>

static struct rt_watchdog_ops wdt_ops;
static rt_watchdog_t watchdog;
hal_wdt_config_t wdt_config;

static void system_wdt_occur(hal_wdt_reset_status_t mode)
{
   if(HAL_WDT_TIMEOUT_RESET == mode)
      DevFail("Watchdog timeout");
   else
      DevFail("Invalid SW watchdog");
}

static rt_err_t wdt_init(rt_watchdog_t *wdt)
{
    /* start watchdog, timeout is 30s. Feed it at IdleHook
    when watchdog timeout occurs, then reset system */

    wdt_config.mode = HAL_WDT_MODE_RESET; /* reset mode */
    wdt_config.seconds = SYSTEM_HANG_CHECK_TIMEOUT_DURATION;
    
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    if (hal_wdt_init(&wdt_config) != HAL_WDT_STATUS_OK)
    {
        LOG_E("wdt init failed.");
        return -RT_ERROR;
    }

    hal_wdt_register_callback(system_wdt_occur);
    if (hal_wdt_enable(HAL_WDT_ENABLE_MAGIC) != HAL_WDT_STATUS_OK)
    {
        LOG_E("wdt init failed.");
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t wdt_control(rt_watchdog_t *wdt, int cmd, void *arg)
{
    switch (cmd)
    {
        /* feed the watchdog */
    case RT_DEVICE_CTRL_WDT_KEEPALIVE:
        hal_wdt_feed(HAL_WDT_FEED_MAGIC);
        break;
        /* set watchdog timeout */
    case RT_DEVICE_CTRL_WDT_SET_TIMEOUT:
        if (hal_wdt_get_enable_status())
        {
            wdt_set_length((*((uint32_t *)arg) + 999) / 1000); /* ms to s */
        }
        else
        {
            LOG_E("wdt set timeout failed. wdt is not inited!");
            return -RT_ERROR;
        }
        break;
    case RT_DEVICE_CTRL_WDT_STOP:
        if (HAL_WDT_STATUS_OK != hal_wdt_disable(HAL_WDT_DISABLE_MAGIC))
        {
            LOG_E("Disable wdt failed!");
            return -RT_ERROR;
        }
        LOG_I("disable wdt");
        break;
    case RT_DEVICE_CTRL_WDT_START:
        if (hal_wdt_enable(HAL_WDT_ENABLE_MAGIC) != HAL_WDT_STATUS_OK)
        {
            LOG_E("wdt enable failed.");
            return -RT_ERROR;
        }
        LOG_I("enable wdt");
        break;
    default:
        LOG_E("no handle for wdt");
        return -RT_ERROR;
    }
    return RT_EOK;
}

int rt_wdt_init(void)
{
    wdt_ops.init = &wdt_init;
    wdt_ops.control = &wdt_control;
    watchdog.ops = &wdt_ops;

    /* register watchdog device */
    if (rt_hw_watchdog_register(&watchdog, "wdt", RT_DEVICE_FLAG_DEACTIVATE, RT_NULL) != RT_EOK)
    {
        LOG_E("wdt device register failed.");
        return -RT_ERROR;
    }
    return RT_EOK;
}
INIT_BOARD_EXPORT(rt_wdt_init);

#endif /* RT_USING_WDT */
