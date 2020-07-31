/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-09     MurphyZhao        first version
 */

#include "drv_common.h"
#include "board.h"

#include "mt2625.h"
#include "hal_clock_internal.h"
#include "hal_nvic.h"

#include "portmacro.h"
#include "hal_gpt.h"

#include <stdint.h>

#ifdef RT_USING_SERIAL
#include "drv_usart.h"
#endif

#ifdef RT_USING_PIN
#include "drv_gpio.h"
#endif

#ifdef RT_USING_FINSH
#include <finsh.h>
static void reboot(uint8_t argc, char **argv)
{
    extern void exception_reboot(void);
    exception_reboot();
}
FINSH_FUNCTION_EXPORT_ALIAS(reboot, __cmd_reboot, Reboot System);
#endif /* RT_USING_FINSH */

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    rt_tick_increase();
}

/**
 * This function will delay for some us.
 *
 * @param us the delay time of us
 */
void rt_hw_us_delay(rt_uint32_t us)
{
    extern hal_gpt_status_t hal_gpt_delay_us(uint32_t us);
    hal_gpt_delay_us(us);
}

/**
 * This function will initial STM32 board.
 */
RT_WEAK void rt_hw_board_init()
{
    /* System clock initialization */
    extern void SystemClock_Config(void);
    extern void SystemCoreClockUpdate(void);
    SystemClock_Config();    /* in sysinit.c */
    SystemCoreClockUpdate(); /* in sysinit.c */

    extern void os_gpt_init(uint32_t ms);
    os_gpt_init(portTICK_PERIOD_MS); /* 1tick = 10ms */

    /* USART driver initialization is open by default */
#ifdef RT_USING_SERIAL
    rt_hw_usart_init();
#endif

    /* Set the shell console output device */
#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

    /* Heap initialization */
#if defined(RT_USING_HEAP)
    rt_system_heap_init((void *)RTT_HEAP_BEGIN, (void *)RTT_HEAP_END);
    mt_noncached_heap_init();
#endif

    /* Pin driver initialization is open by default */
#ifdef RT_USING_PIN
    rt_hw_pin_init();
#endif

    extern int system_init(void);
    system_init();

    /* Board underlying hardware initialization */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
