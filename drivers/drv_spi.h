/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-08-08     MurphyZhao        first version
 */

#ifndef  __DRV_SPI_H__
#define  __DRV_SPI_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

struct mt2625_spi_cs
{
    uint16_t cs_pin;      /* hal_gpio_pin_t */
    uint16_t cs_pin_func; /* defined in hal_pinmux_define.h */
};

int drv_spi_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __DRV_SPI_H__ */
