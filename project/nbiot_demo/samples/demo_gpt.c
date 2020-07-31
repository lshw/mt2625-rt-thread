/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-16     MurphyZhao        first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#if defined(RT_USING_HWTIMER) && defined(BSP_USING_GPT_TIMER) && defined(NB_GPT_DEMO)

#define LOG_TAG              "demo.gpt"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define HWTIMER_DEV_NAME   "gpt1"

static rt_uint8_t cb_cnt = 0;

static rt_err_t timeout_cb(rt_device_t dev, rt_size_t size)
{
    rt_kprintf("This message is from hwtimer timeout callback fucntion!\n");
    cb_cnt++;

    return 0;
}

static int mtk_hwtimer_sample(int argc, char *argv[])
{
    rt_err_t ret = RT_EOK;
    rt_hwtimerval_t timeout_s;      /* the timeout value of timer */
    rt_device_t hw_dev = RT_NULL;   /* hardware timer handler */
    rt_hwtimer_mode_t mode;         /* the mode of timer */
    char dev_name[RT_NAME_MAX+1];

    if (argc > 1)
    {
        rt_strncpy(dev_name, (const char *)(argv[1]), sizeof(dev_name));
    }
    else
    {
        rt_strncpy(dev_name, HWTIMER_DEV_NAME, sizeof(dev_name));
    }
    
    LOG_I("Will test hwtimer <%s>", dev_name);

    /* find 'dev_name' device */
    hw_dev = rt_device_find(dev_name);
    if (hw_dev == RT_NULL)
    {
        LOG_E("hwtimer sample run failed! can't find %s device!", dev_name);
        return -RT_ERROR;
    }

    /* open device with read and write mode */
    ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        LOG_E("open <%s> device failed!", dev_name);
        return ret;
    }

    /* setting the callback function of timeout */
    rt_device_set_rx_indicate(hw_dev, timeout_cb);

    /* set timer mode to periodic timer */
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        LOG_E("set mode failed! ret is %d", ret);
        ret = -RT_ERROR;
        goto __exit;
    }

    /* set timer timeout value to 5S and start timer */
    timeout_s.sec = 1;      /* second */
    timeout_s.usec = 0;     /* microsecond */

    if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        LOG_E("set timeout value failed");
        ret = -RT_ERROR;
        goto __exit;
    }

    /* delay 3500ms */
    rt_thread_mdelay(3500);

    /* read the current value of the timer */
    rt_device_read(hw_dev, 0, &timeout_s, sizeof(timeout_s));
    LOG_I("Read: sec = %d, usec = %d; cb_cnt:%d", timeout_s.sec, timeout_s.usec, cb_cnt);
    rt_thread_mdelay(5000);
    rt_device_read(hw_dev, 0, &timeout_s, sizeof(timeout_s));
    LOG_I("Read: sec = %d, usec = %d; cb_cnt:%d", timeout_s.sec, timeout_s.usec, cb_cnt);

__exit:
    rt_device_control(hw_dev, HWTIMER_CTRL_STOP, RT_NULL);
    rt_device_close(hw_dev);
    LOG_I("hwtimer <%s> test end, will stop timer and close the device.", dev_name);

    return ret;
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(mtk_hwtimer_sample, demo_gpt, hwtimer test);
#endif /* FINSH_USING_MSH */

#endif /* RT_USING_HWTIMER && BSP_USING_GPT_TIMER && NB_GPT_DEMO */
