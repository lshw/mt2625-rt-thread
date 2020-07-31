/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-13     MurphyZhao        first version
 */

#ifndef  __DRV_WDT_H__
#define  __DRV_WDT_H__

#include "hal_wdt.h"

#ifdef __cplusplus
extern "C" {
#endif

int rt_wdt_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __DRV_WDT_H__ */