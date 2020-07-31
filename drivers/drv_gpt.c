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

#ifdef BSP_USING_GPT_TIMER

#ifdef RT_USING_HWTIMER

#include <rtdevice.h>
#include "drv_gpt.h"
#include "hal_gpt_internal.h"

#define DRV_DEBUG

#ifdef DRV_DEBUG

#define DBG_TAG               "drv.gpt"
#define DBG_LVL               DBG_LOG
#else
#define DBG_LVL               DBG_INFO
#endif /* DRV_DEBUG */
#include <rtdbg.h>

#if !defined(BSP_USING_GPT0) && \
    !defined(BSP_USING_GPT1) && \
    !defined(BSP_USING_GPT2)

#error "Please select one gpt timer first!"

#endif

#define count2ms(cnt) (cnt * 1000 / 32768)
extern gpt_context_t g_gpt_context[HAL_GPT_MAX_PORT];

struct mt2625_hwtimer
{
    rt_hwtimer_t timer_device;
    hal_gpt_port_t gpt_port;
    IRQn_Type irqn;
    rt_uint32_t timeout;
    char *name;
};

static struct mt2625_hwtimer mt2625_hwtimer_sets[] =
{
#ifdef BSP_USING_GPT0
    {
        .gpt_port = HAL_GPT_0, /* used for deep sleep */
        .irqn = GPT_IRQn,
        .name = "gpt0"
    },
#endif

#ifdef BSP_USING_GPT1
    {
        .gpt_port = HAL_GPT_1,
        .irqn = GPT_IRQn,
        .name = "gpt1"
    },
#endif

#ifdef BSP_USING_GPT2
    {
        .gpt_port = HAL_GPT_2,
        .irqn = GPT_IRQn,
        .name = "gpt2"
    }
#endif
};

/* Callback function. This function should be registered with #hal_gpt_register_callback(). */
static void user_gpt_callback (void *user_data)
{
    RT_ASSERT(user_data != RT_NULL);
    struct mt2625_hwtimer *gpt_tm_device = (struct mt2625_hwtimer *)user_data;
    rt_device_hwtimer_isr(&gpt_tm_device->timer_device);
}

static void timer_init(struct rt_hwtimer_device *timer, rt_uint32_t state)
{
    struct mt2625_hwtimer *gpt_tm_device;
    hal_gpt_running_status_t running_status;
    hal_gpt_status_t status;

    RT_ASSERT(timer != RT_NULL);
    if (state)
    {
        gpt_tm_device = (struct mt2625_hwtimer *)timer;

        /* Get the running status to check if this port is in use or not. */
        if (HAL_GPT_STATUS_OK != hal_gpt_get_running_status(HAL_GPT_1, &running_status))
        {
            /* Handle the error, if the timer is running. */
            LOG_E("Get gpt (%d) running status error. Skipped init!", gpt_tm_device->gpt_port);
            return;
        }

        if (running_status == HAL_GPT_RUNNING)
        {
            LOG_W("gpt (%d) is running. Skipped init!", gpt_tm_device->gpt_port);
            return;
        }

        if ((status = hal_gpt_init(gpt_tm_device->gpt_port)) != HAL_GPT_STATUS_OK)
        {
            if (g_gpt_context[gpt_tm_device->gpt_port].has_initilized == true)
            {
                LOG_W("gpt (%d) have been inited!");
            }
            else
            {
                LOG_E("gpt (%d) init failed! Error: %d", gpt_tm_device->gpt_port, status);
            }
            return;
        }

        /* Register a user callback. */
        hal_gpt_register_callback(gpt_tm_device->gpt_port, user_gpt_callback, gpt_tm_device);
    }
}

static rt_err_t timer_start(rt_hwtimer_t *timer, rt_uint32_t timeout, rt_hwtimer_mode_t opmode)
{
    rt_err_t result = RT_EOK;
    struct mt2625_hwtimer *gpt_tm_device;

    RT_ASSERT(timer != RT_NULL);

    gpt_tm_device = (struct mt2625_hwtimer *)timer;
    gpt_tm_device->timeout = count2ms(timeout);

    LOG_D("freq: %d, timeout:%d, ms:%d\n", timer->freq, timeout, gpt_tm_device->timeout);

    if (opmode == HWTIMER_MODE_ONESHOT)
    {
        /* set timer to single mode */
        /* Set timeout, set oneshot mode and start timer. */
        if (hal_gpt_start_timer_ms(gpt_tm_device->gpt_port, gpt_tm_device->timeout, HAL_GPT_TIMER_TYPE_ONE_SHOT) != HAL_GPT_STATUS_OK)
        {
            return -RT_ERROR;
        }
    }
    else if (opmode == HWTIMER_MODE_PERIOD)
    {
        if (hal_gpt_start_timer_ms(gpt_tm_device->gpt_port, gpt_tm_device->timeout, HAL_GPT_TIMER_TYPE_REPEAT) != HAL_GPT_STATUS_OK)
        {
            return -RT_ERROR;
        }
    }

    return result;
}

static void timer_stop(rt_hwtimer_t *timer)
{
    struct mt2625_hwtimer *gpt_tm_device;
    RT_ASSERT(timer != RT_NULL);

    gpt_tm_device = (struct mt2625_hwtimer *)timer;

    /* stop timer */
    hal_gpt_stop_timer(gpt_tm_device->gpt_port);
}

static rt_err_t timer_ctrl(rt_hwtimer_t *timer, rt_uint32_t cmd, void *arg)
{
    struct mt2625_hwtimer *gpt_tm_device;
    rt_err_t result = RT_EOK;
    rt_uint32_t freq;

    RT_ASSERT(timer != RT_NULL);
    RT_ASSERT(arg != RT_NULL);

    gpt_tm_device = (struct mt2625_hwtimer *)timer;
    rt_hwtimer_mode_t opmode;

    switch (cmd)
    {
    case HWTIMER_CTRL_FREQ_SET:
        /* set timer frequence */
        freq = *((rt_uint32_t *)arg);
        LOG_W("HWTIMER_CTRL_FREQ_SET not implemented! freq: %d", freq);
        break;
    case HWTIMER_CTRL_MODE_SET:
        /* stop timer */
        if (hal_gpt_stop_timer(gpt_tm_device->gpt_port) != HAL_GPT_STATUS_OK)
        {
            LOG_E("Stop timer [%d] failed!", gpt_tm_device->gpt_port);
            result = -RT_ENOSYS;
            return result;
        }
        
        opmode = *((rt_hwtimer_mode_t *)arg);
        if (timer_start(timer, gpt_tm_device->timeout, opmode) != HAL_GPT_STATUS_OK)
        {
            LOG_E("Start timer [%d] failed!", gpt_tm_device->gpt_port);
            result = -RT_ENOSYS;
            return result;
        }
        break;
    default:
        result = -RT_ENOSYS;
        break;
    }

    return result;
}

extern uint32_t gpt_current_count(GPT_REGISTER_T *gpt);
extern GPT_REGISTER_T *gp_gpt[HAL_GPT_MAX_PORT];
static rt_uint32_t timer_counter_get(rt_hwtimer_t *timer)
{
    struct mt2625_hwtimer *gpt_tm_device;

    RT_ASSERT(timer != RT_NULL);
    gpt_tm_device = (struct mt2625_hwtimer *)timer;

    return gpt_current_count(gp_gpt[gpt_tm_device->gpt_port]);
}

static struct rt_hwtimer_info _info = 
{
    .maxfreq = 32768, /* 32.768KHZ */
    .minfreq = 32768,
    .maxcnt  = 0xFFFFFFFF,
    .cntmode = HWTIMER_CNTMODE_UP
};

static const struct rt_hwtimer_ops _ops =
{
    .init = timer_init,
    .start = timer_start,
    .stop = timer_stop,
    .count_get = timer_counter_get,
    .control = timer_ctrl,
};

int mt2625_hwtimer_init(void)
{
    int i = 0;
    int result = RT_EOK;

    _info.maxcnt = gpt_convert_ms_to_32k_count(HAL_GPT_MAXIMUM_MS_TIMER_TIME);

    for (i = 0; i < sizeof(mt2625_hwtimer_sets) / sizeof(mt2625_hwtimer_sets[0]); i++)
    {
        mt2625_hwtimer_sets[i].timer_device.info = &_info;
        mt2625_hwtimer_sets[i].timer_device.ops  = &_ops;
        if (rt_device_hwtimer_register(&mt2625_hwtimer_sets[i].timer_device, mt2625_hwtimer_sets[i].name, &mt2625_hwtimer_sets[i]) == RT_EOK)
        {
            LOG_D("%s register success", mt2625_hwtimer_sets[i].name);
        }
        else
        {
            LOG_E("%s register failed", mt2625_hwtimer_sets[i].name);
            result = -RT_ERROR;
        }
    }

    return result;
}
#ifdef RT_USING_COMPONENTS_INIT
INIT_BOARD_EXPORT(mt2625_hwtimer_init);
#endif

#endif /* RT_USING_HWTIMER */

#endif /* BSP_USING_GPT_TIMER */
