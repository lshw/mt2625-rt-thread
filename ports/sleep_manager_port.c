/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-24     MurphyZhao   First version
 */

#include "rtthread.h"

#include "FreeRTOS.h"

/* device.h includes */
#include "hal_platform.h"
#include "mt2625.h"

#include "hal_feature_config.h"
#include "hal_sleep_manager.h"
#include "hal_sleep_manager_internal.h"
#include "hal_rtc_internal.h"
#include "hal_rtc_external.h"

#ifdef HAL_SLEEP_MANAGER_ENABLED
uint8_t sys_lock_handle = 0xFF;

void mtk_sleep_lock(void)
{
    if (sys_lock_handle == 0xFF)
    {
        sys_lock_handle = hal_sleep_manager_set_sleep_handle("sys_init");
    }
    hal_sleep_manager_acquire_sleeplock(sys_lock_handle, HAL_SLEEP_LOCK_ALL);
}
MSH_CMD_EXPORT_ALIAS(mtk_sleep_lock, sleep_lock, disable sleep);

void mtk_sleep_unlock(void)
{
    if (sys_lock_handle == 0xFF)
    {
        return;
    }
    hal_sleep_manager_release_sleeplock(sys_lock_handle, HAL_SLEEP_LOCK_ALL);
}
MSH_CMD_EXPORT_ALIAS(mtk_sleep_unlock, sleep_unlock, enable sleep);
#endif /* HAL_SLEEP_MANAGER_ENABLED */

#ifdef BSP_USING_SLEEP_MANAGER

extern uint8_t mtk_system_is_ready(void);
extern void tickless_handler(uint32_t xExpectedIdleTime);

void sys_tickless_hook(void)
{
    rt_tick_t xExpectedIdleTime;

    if (mtk_system_is_ready() != 0)
    {
        xExpectedIdleTime = rt_timer_next_timeout_tick() - rt_tick_get();

        if( xExpectedIdleTime >= configEXPECTED_IDLE_TIME_BEFORE_SLEEP )
        {
            rt_enter_critical();
            xExpectedIdleTime = rt_timer_next_timeout_tick() - rt_tick_get();
            if( xExpectedIdleTime >= configEXPECTED_IDLE_TIME_BEFORE_SLEEP )
            {
                tickless_handler(xExpectedIdleTime);
            }
            rt_exit_critical();
        }
    }
}

#if (configUSE_TICKLESS_IDLE == 2)
#ifdef HAL_RTC_MODULE_ENABLED
extern uint32_t systick_backup;
extern void vTaskStepTick( const TickType_t xTicksToJump );

static void prvRestoreSystickFromDeepOrDeeperSleepMode( void )
{
    uint64_t rtc_elapsed_tick;
    uint64_t rtc_sec, rtc_passed_32k;
    uint32_t os_count_per_tick = 1000000/configTICK_RATE_HZ;

    rtc_get_elapsed_tick_sram(true, &rtc_elapsed_tick);

    rtc_sec = rtc_elapsed_tick / 32768;
    rtc_passed_32k = rtc_elapsed_tick % 32768;
    systick_backup += (rtc_sec*configTICK_RATE_HZ + rtc_passed_32k*1000000/32768/os_count_per_tick);

    vTaskStepTick(systick_backup);
}
#else
    #error please enable HAL_RTC_MODULE_ENABLED in project inc/hal_feature_config.h for restore systick from deeper sleep mode.
#endif /* HAL_RTC_MODULE_ENABLED */
#endif /* (configUSE_TICKLESS_IDLE == 2) */

/**
 * check_poweron_restore_systick must before vPortSetupTimerInterrupt 
*/
void check_poweron_restore_systick(void)
{
#if (configUSE_TICKLESS_IDLE == 2)
    #ifdef HAL_RTC_MODULE_ENABLED
    /* Must adjust FreeRTOS tick before vPortSetupTimerInterrupt(), because xTaskGetTickCount is invoked in vPortSetupTimerInterrupt*/
        rtc_power_on_result_t power_on_result = rtc_power_on_result_external();
        if ((power_on_result == DEEP_SLEEP) || (power_on_result == DEEPER_SLEEP))
        {
            /*restore systick from deep or deeper sleep mode*/
            prvRestoreSystickFromDeepOrDeeperSleepMode();
        }
    #endif /* HAL_RTC_MODULE_ENABLED */
#endif /* (configUSE_TICKLESS_IDLE == 2) */
}

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
#ifdef HAL_DWT_MODULE_ENABLED
void vPortCurrentTaskStackOverflowCheck(void)
{
    /* TODO for rt-thread */
}
#else
	#error please enable HAL_DWT_MODULE_ENABLED in project inc/hal_feature_config.h for task stack overflow check.
#endif /* HAL_DWT_MODULE_ENABLED */
#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */

#endif /* BSP_USING_SLEEP_MANAGER */
