/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-24     MurphyZhao   First version
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <afnv_typ.h>

#include <rtthread.h>
#include "rt_nb_port.h"

/* Software and hardware information */
const AtIdentificationInfo atIdentificationInfoDefaultInit =
{
    /* atIdentificationInfoInit */
    { /* AtIdentificationText */
        "RealThread",                     /* manufacturer id */
        "NB-IOT Mobile Station",          /* model id        */
        RT_NB_SDK_VERSION,            /* S/W revision id */
        "MT2625_V01",                     /* H/W revision id */
        "Null ID"                         /* global id       */
    }
};

char *rt_nb_manufacturer_id_get(void)
{
    return (char *)atIdentificationInfoDefaultInit.atIdentificationText.manufacturerId;
}

char *rt_nb_model_id_get(void)
{
    return (char *)atIdentificationInfoDefaultInit.atIdentificationText.modelId;
}

char *rt_nb_sw_ver_get(void)
{
    return (char *)atIdentificationInfoDefaultInit.atIdentificationText.swRevision;
}

char *rt_nb_hw_ver_get(void)
{
    return (char *)atIdentificationInfoDefaultInit.atIdentificationText.hwRevision;
}
