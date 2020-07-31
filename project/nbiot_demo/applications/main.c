/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-04-27     MurphyZhao   first version
 */

#include <rtthread.h>
#include <stdio.h>

#include "hal_platform.h"
#include "rt_nb_port.h"

#ifdef RT_USING_NETDEV
#include <arpa/inet.h>
#include <netdev.h>
#endif /* RT_USING_NETDEV */

#define LOG_TAG              "main"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define NB_NETWORK_CARD_NAME "ps"
#define NB_NW_CHECK_CNT      (100)

extern int mtk_sys_init(void);
extern int md_init(void);
extern int mtk_components_init(void);
extern void rs_sdk_start();

static void nb_nw_status_cb(struct netdev *netdev, enum netdev_cb_type type)
{
    switch (type)
    {
    case NETDEV_CB_STATUS_LINK_UP:
        LOG_D("netdev link up cb");
        break;
    case NETDEV_CB_STATUS_LINK_DOWN:
        LOG_D("netdev link down cb");
        break;
    case NETDEV_CB_STATUS_INTERNET_UP:
        LOG_D("netdev internet up cb");
        break;
    case NETDEV_CB_STATUS_INTERNET_DOWN:
        LOG_D("netdev internet down cb");
        break;
    default:
        break;
    }
}

int main(void)
{
    struct netdev *net_dev;

#ifdef HAL_SLEEP_MANAGER_ENABLED
    extern void mtk_sleep_lock(void);
    mtk_sleep_lock();
#endif

    LOG_I("Current sdk version: %s", rt_nb_sw_ver_get());

    net_dev = netdev_get_by_name(NB_NETWORK_CARD_NAME);
    if (!net_dev)
    {
        LOG_E("nbiot network init failed!");
        return -RT_ERROR;
    }

    netdev_set_status_callback(net_dev, nb_nw_status_cb);

    if (rt_nb_ril_init() != RT_EOK)
    {
        return -RT_ERROR;
    }

    /* First  step: init system (eg: clock/systick...) */
    mtk_sys_init();
    /* Second step: init md subsystem */
    md_init();
    /* Third  step: init others services (eg: tcpip/mux/ota...) */
    mtk_components_init();

    /* check network status */
    {
        int retry_cnt;

        for (retry_cnt = 0; retry_cnt < NB_NW_CHECK_CNT; retry_cnt ++)
        {
            if (netdev_is_link_up(net_dev))
            {
                break;
            }

            rt_thread_mdelay(1000);
        }

        if (retry_cnt == NB_NW_CHECK_CNT)
        {
            LOG_E("nbiot network is not linked up");
            return -RT_ERROR;
        }
    }

    /* sync nbiot general config to memory */
    if (rt_nb_cfg_sync() != 0)
    {
        LOG_E("nb cfg sync failed!");
        return -RT_ERROR;
    }

    /* user code */
    {

        /* user code */
    }

    return RT_EOK;
}
