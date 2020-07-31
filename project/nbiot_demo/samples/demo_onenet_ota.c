/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-11-20     MurphyZhao        first version
 */

#include <rtthread.h>

#ifdef NB_ONENET_OTA_DEMO

#ifdef __RS_FOTA_SUPPORT__
#include "rs_sdk_api.h"

#define LOG_TAG              "demo.ota"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

static void nb_onenet_ota_demo(void)
{
    rs_sdk_start();
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(nb_onenet_ota_demo, demo_ota, nbiot onenet ota start);
#endif /* FINSH_USING_MSH */

static void nb_ota_firmware_check(void)
{
    LOG_D("Current ver: %s", (char *)rs_sdk_version());

    LOG_D("Start check new firmware...");
    if (0 == rs_sdk_check(0))
    {
        LOG_D("ota check ok");
    }
    LOG_D("Check new firmware end");
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(nb_ota_firmware_check, demo_ota_fm_check, check new firmware);
#endif /* FINSH_USING_MSH */

#endif /* __RS_FOTA_SUPPORT__ */
#endif /* NB_ONENET_OTA_DEMO */
