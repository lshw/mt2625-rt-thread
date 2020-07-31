/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      SummerGift   first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
// #include <stm32f4xx.h>
#include "drv_common.h"
// #include "drv_gpio.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "memory_attribute.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MT2625_SRAM_SIZE        (2048) /* Kbytes */
#define MT2625_SRAM_END         (0x10000000 + MT2625_SRAM_SIZE * 1024)

#if defined(RT_USING_HEAP)
#if( configAPPLICATION_ALLOCATED_HEAP == 1 )

#if 1
/* in mtk_main.c */
extern ATTR_NONINIT_DATA_IN_RAM uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

#define RTT_HEAP_BEGIN      ((void *)&ucHeap[0])
#define RTT_HEAP_END        ((void *)&ucHeap[configTOTAL_HEAP_SIZE -1])

#else /* for test */

extern uint32_t _user_cached_heap_start;
extern uint32_t _user_cached_heap_end;

#define RTT_HEAP_BEGIN      ((void *)&_user_cached_heap_start)
#define RTT_HEAP_END        ((void *)&_user_cached_heap_end)
#endif

#endif /* configAPPLICATION_ALLOCATED_HEAP == 1 */
#endif /* RT_USING_HEAP */

#include "hal_platform.h"

/**
 * pin mapping for M5311
 * M5311 pin index   | MT2625 pin number | M5311 pin func | MT2625 pin func    | Usage
*/
#define NB_PIN_1     HAL_GPIO_5          /* UART2_TXD     | UART0_TXD_MT2625   | GKI && Finsh cmd */
#define NB_PIN_2     HAL_GPIO_2          /* UART2_RXD     | UART0_RXD_MT2625   | GKI && Finsh cmd */
#define NB_PIN_3     HAL_GPIO_8          /* SPI_SS        | SPI_CS_MT2625      | Normal */
#define NB_PIN_4     HAL_GPIO_9          /* SPI_MISO      | SPI_MISO_MT2625    | Normal */
#define NB_PIN_5     HAL_GPIO_10         /* SPI_MOSI      | SPI_MOSI_MT2625    | Normal */
#define NB_PIN_6     HAL_GPIO_11         /* SPI_SCLK      | SPI_SCK_MT2625     | Normal */
#define NB_PIN_7     HAL_GPIO_18         /* UART1_CTS     | UART1_CTS_MT2625   | Normal */
#define NB_PIN_8     HAL_GPIO_23         /* UART1_RTS     | UART1_RTS_MT2625   | Normal */
#define NB_PIN_9     HAL_GPIO_17         /* UART1_TXD     | UART1_TXD_MT2625   | AT     */
#define NB_PIN_10    HAL_GPIO_16         /* UART1_RXD     | UART1_RXD_MT2625   | AT     */
#define NB_PIN_11    HAL_GPIO_35         /* USIM_DETECT   | SIM_DET_MT2625     | Normal */
// #define NB_PIN_12    K11              /* USIM_RESET    | SIM_SRST_MT2625    | System used */
// #define NB_PIN_13    L11              /* USIM_CLK      | SIM_SCLK_MT2625    | System used */
// #define NB_PIN_14    K10              /* USIM_DATA     | SIM_SIO_MT2625     | System used */
// #define NB_PIN_15    L10              /* USIM_VCC      | VSIM_MT2625        | System used */
#define NB_PIN_16    HAL_GPIO_27         /* WAKEUP_OUT    | MD_WAKEUP_MT2625   | Normal      */
// #define NB_PIN_17    J3               /* RESET         | PMU_SYSRSTB_MT2625 | System used */
// #define NB_PIN_18    J6               /* WAKEUP_IN     | RTC_EINT_MT2625    | System used */
// #define NB_PIN_19    K5               /* POWER_ON/OFF  | PMU_PWRKEY_MT2625  | System used */
// #define NB_PIN_20    None             /* VDD           | NONE               | None        */
#define NB_PIN_21    HAL_GPIO_26         /* SYS_STATE     | STATUS_MT2625      | Normal      */
// #define NB_PIN_22    None             /* GND           | NONE               | None        */
// #define NB_PIN_23    None             /* GND           | NONE               | None        */
#define NB_PIN_24    HAL_GPIO_31         /* GNSS_ANT      | ADC1_MT2625        | Normal      */
// #define NB_PIN_25    None             /* GND           | NONE               | None        */
// #define NB_PIN_26    None             /* GND           | NONE               | None        */
// #define NB_PIN_27    None             /* ANT0          | ANT_MT2625         | System used */
// #define NB_PIN_28    None             /* GND           | NONE               | None        */
// #define NB_PIN_29    None             /* GND           | NONE               | None        */
// #define NB_PIN_30    None             /* GND           | NONE               | None        */
// #define NB_PIN_31    None             /* VCC1          | NONE               | None        */
// #define NB_PIN_32    None             /* VCC2          | NONE               | None        */
#define NB_PIN_33    HAL_GPIO_22         /* RTCPOWER      | MD_GPIO3_MT2625    | Normal      */
#define NB_PIN_34    HAL_GPIO_19         /* GPIO          | MD_GPIO0_MT2625    | Normal      */
#define NB_PIN_35    HAL_GPIO_20         /* GPIO          | MD_GPIO1_MT2625    | Normal      */
#define NB_PIN_36    HAL_GPIO_6          /* SCL           | I2C0_SCL_MT2625    | Normal      */
#define NB_PIN_37    HAL_GPIO_7          /* SDA           | I2C0_SDA_MT2625    | Normal      */
#define NB_PIN_38    HAL_GPIO_30         /* ADC           | ADC0_MT2625        | Normal      */
#define NB_PIN_39    HAL_GPIO_29         /* RESERVED      | AP_READY_MT2625    | Normal      */
#define NB_PIN_40    HAL_GPIO_21         /* RESERVED      | MD_GPIO2_MT2625    | Normal      */

#define NB_PIN_EXT_1 HAL_GPIO_3          /* Extended      | HAL_GPIO_3_UART2_RXD  | Retain   */
#define NB_PIN_EXT_2 HAL_GPIO_4          /* Extended      | HAL_GPIO_4_UART2_TXD  | Retain   */
#define NB_PIN_EXT_3 HAL_GPIO_33         /* Extended      | HAL_GPIO_33_UART3_RXD | Retain   */
#define NB_PIN_EXT_4 HAL_GPIO_34         /* Extended      | HAL_GPIO_34_UART3_TXD | Retain   */

void mt_noncached_heap_init(void);
void *mt_noncached_malloc(rt_size_t size);
void mt_noncached_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif

