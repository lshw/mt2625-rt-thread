/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-08-12     MurphyZhao        first version
 */

#include "rtthread.h"

#if defined(BSP_USING_ONCHIP_RTC)

#ifdef RT_USING_RTC

#include <rtdevice.h>
#include "drv_rtc.h"
#include "hal_rtc.h"
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_gpio.h"

// #define DRV_DEBUG

#ifdef DRV_DEBUG

#define DBG_TAG               "drv.rtc"
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_ERROR
#endif /* DRV_DEBUG */
#include <rtdbg.h>

static struct rt_device rtc;

static rt_err_t drv_rtc_control(rt_device_t dev, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    hal_rtc_status_t status = HAL_RTC_STATUS_OK;
    hal_rtc_time_t time;
    time_t time_stamp;
    struct tm *tm_old;
    struct tm tm_new;
    RT_ASSERT(dev != RT_NULL);
    RT_ASSERT(args != RT_NULL);

    LOG_D("rtc control, cmd:%d", cmd);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        if(HAL_RTC_STATUS_OK != (status = hal_rtc_get_time(&time)))
        {
            LOG_E("hal rtc set time error, code:%d", status);
            return -RT_ERROR;
        }

        // The user has to define the base year and the RTC year is defined
        // as an offset. For example, define the base year as 2000 and assign 15 to the RTC year to represent the year of 2015.
        rt_memset(&tm_new, 0x0, sizeof(tm_new));
        tm_new.tm_year = time.rtc_year + 100;
        tm_new.tm_mon  = time.rtc_mon - 1;
        tm_new.tm_mday = time.rtc_day;
        tm_new.tm_hour = time.rtc_hour;
        tm_new.tm_min  = time.rtc_min;
        tm_new.tm_sec  = time.rtc_sec;

        *(rt_uint32_t *)args = mktime(&tm_new);
        LOG_D("RTC: get rtc_time %x\n", *(rt_uint32_t *)args);
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:

        time_stamp = (time_t)(*(rt_uint32_t *)args);

        tm_old = localtime(&time_stamp);
        if (tm_old->tm_year < 100)
        {
            return -RT_ERROR;
        }

        // The user has to define the base year and the RTC year is defined
        // as an offset. For example, define the base year as 2000 and assign 15 to the RTC year to represent the year of 2015.
        rt_memset(&time, 0x0, sizeof(time));
        time.rtc_year = tm_old->tm_year - 100; /* 1900 + 100 = 2000 */
        time.rtc_mon  = tm_old->tm_mon + 1;
        time.rtc_day  = tm_old->tm_mday;
        time.rtc_hour = tm_old->tm_hour;
        time.rtc_min  = tm_old->tm_min;
        time.rtc_sec  = tm_old->tm_sec;
        time.rtc_milli_sec = 0;
        time.rtc_week = tm_old->tm_wday;

        // Set the RTC current time.
        if(HAL_RTC_STATUS_OK != (status = hal_rtc_set_time(&time)))
        {
            result = -RT_ERROR;
            LOG_E("hal rtc set time error, code:%d", status);
        }

        LOG_D("RTC: set rtc time %x", *(rt_uint32_t *)args);
        break;
    }

    return result;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops rtc_ops =
{
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    RT_NULL,
    drv_rtc_control
};
#endif

int rt_hw_rtc_init(void)
{
    rt_err_t result = RT_EOK;
    // hal_rtc_status_t status = HAL_RTC_STATUS_OK;
    rt_device_t device = &rtc;
    LOG_D("Register rtc device");

    /**
     * Notice:
     * Can not init rtc driver by `hal_rtc_init` interface here!
     * Please fellow mtk sdk init.
     * If call `hal_rtc_init` here, a assert will happened!
    */
#if 0 /* Not enable it */
    if ((status = hal_rtc_init()) != HAL_RTC_STATUS_OK)
    {
        LOG_E("hal rtc init failed. Error:%d", status);
        return -RT_ERROR;
    }

    LOG_E("Error poweron mode: %d!", rtc_power_on_result_external());
#endif /* 0 */

    rt_memset(device, 0x0, sizeof(struct rt_device));

#ifdef RT_USING_DEVICE_OPS
    device->ops         = &rtc_ops;
#else
    device->init        = RT_NULL;
    device->open        = RT_NULL;
    device->close       = RT_NULL;
    device->read        = RT_NULL;
    device->write       = RT_NULL;
    device->control     = drv_rtc_control;
#endif
    device->type        = RT_Device_Class_RTC;
    device->rx_indicate = RT_NULL;
    device->tx_complete = RT_NULL;
    device->user_data   = RT_NULL;

    /* register a rtc device, the name must be 'rtc' */
    if ((result = rt_device_register(device, "rtc", RT_DEVICE_FLAG_RDWR)) != RT_EOK)
    {
        LOG_E("rtc device register error. Code: %d", result);
    }
    else
    {
        LOG_D("Register rtc device success.");
    }

    return result;
}
#ifdef RT_USING_COMPONENTS_INIT
INIT_DEVICE_EXPORT(rt_hw_rtc_init);
#endif
#endif /* RT_USING_RTC */
#endif /* BSP_USING_ONCHIP_RTC */
