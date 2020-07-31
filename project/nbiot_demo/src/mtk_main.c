/* Copyright Statement:
 *
 * (C) 2005-2017  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its licensors.
 * Without the prior written permission of MediaTek and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) MediaTek Software
 * if you have agreed to and been bound by the applicable license agreement with
 * MediaTek ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* device.h includes */
#include "mt2625.h"

/* hal includes */
#include "hal.h"
#include "hal_rtc_internal.h"
#include "hal_rtc_external.h"
#include "memory_attribute.h"
#include "hal_dwt.h"

#include "nvdm.h"

#include "sys_init.h"
#include "task_def.h"
#include "tel_conn_mgr_app_api.h"
#include "ril_task.h"
#ifdef MTK_FOTA_ENABLE
#include "fota.h"
#ifdef APB_PROXY_FOTA_CMD_ENABLE
#include "apb_proxy_fota_cmd.h"
#endif
#endif
#ifdef MTK_USB_DEMO_ENABLED
#include "usb.h"
#endif

#ifdef __RS_FOTA_SUPPORT__
#include "rs_sdk_api.h"
#endif

//#include "sensor_demo.h"
#ifdef SENSOR_DEMO
#include "sensor_alg_interface.h"
#endif

#ifdef MTK_GNSS_ENABLE
#include "gnss_app.h"
extern void gnss_demo_main();
#endif

/* for tracing and assert function prototypes */
#include "frhsl.h"
#include "system.h"

/* mux ap includes */
#include "mux_ap.h"
/* ril includes */
#include "ril.h"
/* AP Bridge Proxy inlcudes*/
#include "apb_proxy_task.h"
#include "auto_register.h"

#include "lwip/tcpip.h"
#ifdef MTK_TCPIP_FOR_NB_MODULE_ENABLE
#include "nbnetif.h"
#endif
#include "nidd_gprot.h"
#ifdef MTK_ATCI_APB_PROXY_NETWORK_ENABLE
#include "apb_proxy_nw_cmd_gprot.h"
#endif

#include "lwip/init.h"

#include <rtthread.h>
#include <rthw.h>
#include "board.h"

#ifdef HAL_KEYPAD_MODULE_ENABLED
#include "keypad_custom.h"
#ifdef HAL_SLEEP_MANAGER_ENABLED
#include "hal_sleep_manager.h"
#include "hal_sleep_driver.h"
#include "hal_gpt.h"
#define POWER_KEY_LOCK_SLEEP_IN_SEC 10
const static char *powerkey_lock_sleep_name = "Powerkey_main";
static uint8_t powerkey_lock_sleep_handle;
static bool power_key_timer = false;
uint32_t timer_handle;
#endif
#endif

#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
ATTR_NONINIT_DATA_IN_RAM uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

extern void md_init_phase_2(void);
extern void simat_proxy_init();

#ifdef MTK_CTIOT_SUPPORT
extern void lwm2m_restore_result_callback(bool result, void *user_data);
#endif

#ifdef MTK_COAP_SUPPORT
#ifdef MTK_COAP_AT_CMD_ENABLE
extern void nw_coap_init(void);
#endif
#endif

struct assert_user_var_t global_assert_user_var_t = {0};

extern void mtk_system_ready(void);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
//#define FREERTOS_TEST

/* Private variables ---------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

#if !defined (MTK_DEBUG_LEVEL_NONE)
#if defined(NB_AT_CMD_ENABLE) && defined(NB_USING_ATCI)
LOG_CONTROL_BLOCK_DECLARE(atci_serialport);
LOG_CONTROL_BLOCK_DECLARE(atcmd);
#endif /* NB_AT_CMD_ENABLE */
LOG_CONTROL_BLOCK_DECLARE(common);
LOG_CONTROL_BLOCK_DECLARE(hal);
#ifdef MTK_HTTPCLIENT_SSL_ENABLE
LOG_CONTROL_BLOCK_DECLARE(httpclient);
#endif
#ifdef NB_USING_IPERF
LOG_CONTROL_BLOCK_DECLARE(iperf);
#endif
#ifdef NB_USING_PING
LOG_CONTROL_BLOCK_DECLARE(ping);
#endif

#if defined(NB_AT_CMD_ENABLE) && defined(NB_USING_ATCI)
LOG_CONTROL_BLOCK_DECLARE(RTC_ATCI);
#endif /* NB_AT_CMD_ENABLE */

#ifdef MTK_FOTA_ENABLE
LOG_CONTROL_BLOCK_DECLARE(fota_http_dl);
LOG_CONTROL_BLOCK_DECLARE(fota);
#endif
LOG_CONTROL_BLOCK_DECLARE(apb_proxy);
LOG_CONTROL_BLOCK_DECLARE(mux_ap);
#ifdef MTK_HTTPCLIENT_SSL_ENABLE
LOG_CONTROL_BLOCK_DECLARE(auto_reg);
#endif


log_control_block_t *syslog_control_blocks[] = {
#if defined(NB_AT_CMD_ENABLE) && defined(NB_USING_ATCI)
    &LOG_CONTROL_BLOCK_SYMBOL(atci_serialport),
    &LOG_CONTROL_BLOCK_SYMBOL(atcmd),
#endif /* NB_AT_CMD_ENABLE */
    &LOG_CONTROL_BLOCK_SYMBOL(common),
    &LOG_CONTROL_BLOCK_SYMBOL(hal),
#ifdef MTK_HTTPCLIENT_SSL_ENABLE
    &LOG_CONTROL_BLOCK_SYMBOL(httpclient),
#endif
#ifdef NB_USING_IPERF
    &LOG_CONTROL_BLOCK_SYMBOL(iperf),
#endif
#ifdef NB_USING_PING
    &LOG_CONTROL_BLOCK_SYMBOL(ping),
#endif
#if defined(NB_AT_CMD_ENABLE) && defined(NB_USING_ATCI)
    &LOG_CONTROL_BLOCK_SYMBOL(RTC_ATCI),
#endif /* NB_AT_CMD_ENABLE */
#ifdef MTK_FOTA_ENABLE
    &LOG_CONTROL_BLOCK_SYMBOL(fota_http_dl),
    &LOG_CONTROL_BLOCK_SYMBOL(fota),
#endif
    &LOG_CONTROL_BLOCK_SYMBOL(apb_proxy),
    &LOG_CONTROL_BLOCK_SYMBOL(mux_ap),
#ifdef MTK_HTTPCLIENT_SSL_ENABLE
    &LOG_CONTROL_BLOCK_SYMBOL(auto_reg),
#endif
    NULL
};

static void syslog_config_save(const syslog_config_t *config)
{
    nvdm_status_t status;
    char *syslog_filter_buf;

    syslog_filter_buf = (char*)pvPortMalloc(SYSLOG_FILTER_LEN);
    configASSERT(syslog_filter_buf != NULL);
    syslog_convert_filter_val2str((const log_control_block_t **)config->filters, syslog_filter_buf);
    status = nvdm_write_data_item("common",
                                  "syslog_filters",
                                  NVDM_DATA_ITEM_TYPE_STRING,
                                  (const uint8_t *)syslog_filter_buf,
                                  strlen(syslog_filter_buf));
    vPortFree(syslog_filter_buf);
    LOG_I(common, "syslog config save, status=%d", status);
}

static uint32_t syslog_config_load(syslog_config_t *config)
{
    uint32_t sz = SYSLOG_FILTER_LEN;
    char *syslog_filter_buf;

    syslog_filter_buf = (char*)pvPortMalloc(SYSLOG_FILTER_LEN);
    configASSERT(syslog_filter_buf != NULL);
    if (nvdm_read_data_item("common", "syslog_filters", (uint8_t*)syslog_filter_buf, &sz) == NVDM_STATUS_OK) {
        syslog_convert_filter_str2val(config->filters, syslog_filter_buf);
    } else {
        /* popuplate the syslog nvdm with the image setting */
        syslog_config_save(config);
    }
    vPortFree(syslog_filter_buf);
    return 0;
}
#endif

void big_assert(void);

void AssertHandlerNoReturn(unsigned flags,...)
{
	/* This is desperate trick to possible store registers before they are cleared/overrun/used */
	asm volatile ("nop" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12" );

    char *module_name = NULL;
    int line_number = 0;
    rt_base_t level;
    //char *function_name = NULL;
    //char *user_msg = NULL;
    uint16_t conditions = 0;
    va_list arg_list;
    //char trace_buffer[80];

    level = rt_hw_interrupt_disable();
    level = level; /* avoid build warning */

    // go to beginning of variable argument list
    va_start(arg_list, flags);

    // read argumets based on flag bits
    conditions = (uint16_t) flags;
    if (flags & AF_FLAGS_FILE_)
    {
      module_name = va_arg(arg_list, char *);
      line_number = va_arg(arg_list, int);
    }
    if (flags & AF_FLAGS_FUNC_)
    {
      //function_name = va_arg(arg_list, char *);
    }
    if (flags & AF_FLAGS_COND_)
    {
      conditions |= (1u << 8);
    }
    if (flags & AF_FLAGS_MSG_)
    {
      //user_msg = va_arg(arg_list, char *);
    }
    if (flags & AF_FLAGS_VARS_)
    {
      uint32_t loopc;
      global_assert_user_var_t.user_var_present = true;

      for(loopc = 0; loopc < 3; loopc++)
      {
        global_assert_user_var_t.user_var[loopc] = va_arg(arg_list, unsigned int);
      }
    }
    if (0 == (flags & AF_FLAGS_FATAL_))
    {
      conditions |= (1u << 9);
    }

/*
	// Trace error info to HLS
    FrHslString (0x000A, "\r\n-- AssertHandlerNoReturn called --");
    sprintf(trace_buffer, "\r\n   with Flags = %#04X",conditions);
    FrHslString (0x000A, trace_buffer);

//    M_FrHslPrintf1 (0x9515, DEFAULT_GROUP, "Assert: AssertHandlerNoReturn called. Flags=%{1}8.0b, condition checked=%{1}1.8b, error=%{1}1.9b, raw=%{1}#04X",
//      conditions)
    if (NULL != module_name)
    {
        sprintf(trace_buffer, "\r\n  Module name: %s",module_name);
        FrHslString (0x000A, trace_buffer);
        sprintf(trace_buffer, "\r\n  Line number: %lu",line_number);
        FrHslString (0x000A, trace_buffer);
//        M_FrHslString (0xCAF8, DEFAULT_GROUP, "  Module name: %s", module_name);
//        M_FrHslPrintf2 (0xBB0F, DEFAULT_GROUP, "  Line number: %lu", (uint16_t) (line_number >> 16), (uint16_t) line_number);
    }
    if (NULL != function_name)
    {
        sprintf(trace_buffer, "\r\n  Function name: %s", function_name);
        FrHslString (0x000A, trace_buffer);
//        M_FrHslString (0xCF58, DEFAULT_GROUP, "  Function name: %s", function_name);
    }

//  !!! Need to figure out still how to put this data out, will help debugging when we see this data in HSL traces as well !!!

    if (NULL != user_msg)
    {
        sprintf(trace_buffer, "\r\n  User message: %s", user_msg);
        FrHslString (0x000A, trace_buffer);
//        M_FrHslString (0xC724, DEFAULT_GROUP, "  User message: %s", user_msg);
    }

//        sprintf(trace_buffer, "\r\n  User variables: [0]=%#08lX, [1]=%#08lX, [2]=%#08lX", user_var[0], user_var[1], user_var[2]);
//        FrHslString (0x000A, trace_buffer);
//        M_FrHslPrintf6 (0xA3F8, DEFAULT_GROUP, "  User variables: [0]=%#08lX, [1]=%#08lX, [2]=%#08lX",
//        (uint16_t) (user_var[0] >> 16), (uint16_t) user_var[0],
//        (uint16_t) (user_var[1] >> 16), (uint16_t) user_var[1],
//        (uint16_t) (user_var[2] >> 16), (uint16_t) user_var[2]);
    }
*/

    // call platform assert to create full path towards memory dump
    platform_assert(0, module_name, line_number );

    // this function prototype is defined with noreturn pragma, if function returns to
    // caller system will crash in very mysterious ways.
    for(;;);
}

#define mainCHECK_DELAY ( ( portTickType ) 1000 / portTICK_RATE_MS )


/**
* @brief       Task main function
* @param[in]   pvParameters: Pointer that will be used as the parameter for the task being created.
* @return      None.
*/
/*
static void vTestTask(void *pvParameters)
{
    uint32_t idx = (int)pvParameters;

    portTickType xLastExecutionTime, xDelayTime;
    xLastExecutionTime = xTaskGetTickCount();
    xDelayTime = (1 << idx) * mainCHECK_DELAY;

    while (1) {
        vTaskDelayUntil(&xLastExecutionTime, xDelayTime);
        rt_kprintf("Hello World from %u at %u \r\n", idx, xTaskGetTickCount());
    }
}
*/

#ifdef HAL_KEYPAD_FEATURE_POWERKEY
#ifdef HAL_SLEEP_MANAGER_ENABLED
void powerkey_timer_callback(void *user_data)
{
    /*unlock sleep*/
    hal_sleep_manager_release_sleeplock(powerkey_lock_sleep_handle, HAL_SLEEP_LOCK_ALL);
    /*Stop gpt timer*/
    hal_gpt_sw_stop_timer_ms(timer_handle);
    /*free gpt timer*/
    hal_gpt_sw_free_timer(timer_handle);
    power_key_timer = false;
    log_hal_info("[pwk_main]timer callback done\r\n");
}
#endif

static void keypad_user_powerkey_handler(void)
{
    hal_keypad_status_t ret;
    hal_keypad_powerkey_event_t powekey_event;
    char *string[5] = {"release", "press", "longpress", "repeat", "pmu_longpress"};

    while (1) {
        ret = hal_keypad_powerkey_get_key(&powekey_event);

        /*If an error occurs, there is no key in the buffer*/
        if (ret == HAL_KEYPAD_STATUS_ERROR) {
            //log_hal_info("[pwk_main]powerkey no key in buffer\r\n\r\n");
            break;
        }

#ifdef HAL_SLEEP_MANAGER_ENABLED
        if (powekey_event.state == HAL_KEYPAD_KEY_PRESS || powekey_event.state == HAL_KEYPAD_KEY_LONG_PRESS) {
            if (power_key_timer == false) {
                hal_gpt_status_t ret_gpt;
                /*get timer*/
                ret_gpt = hal_gpt_sw_get_timer(&timer_handle);
                if (ret_gpt != HAL_GPT_STATUS_OK) {
                    log_hal_info("[pwk_main]get timer handle error, ret = %d, handle = 0x%x\r\n",
                                 (unsigned int)ret_gpt,
                                 (unsigned int)timer_handle);
                    return;
                }

                /*lock sleep*/
                hal_sleep_manager_acquire_sleeplock(powerkey_lock_sleep_handle, HAL_SLEEP_LOCK_ALL);
                log_hal_info("[pwk_main]start timer\r\n");
                /*start timer*/
                hal_gpt_sw_start_timer_ms(timer_handle,
                                          POWER_KEY_LOCK_SLEEP_IN_SEC * 1000, //10 sec
                                          (hal_gpt_callback_t)powerkey_timer_callback, NULL);
                power_key_timer = true;
            }
        }
#endif
        log_hal_info("[pwk_main]powerkey data:[%d], state:[%s]\r\n", (int)powekey_event.key_data, (char *)string[powekey_event.state]);
    }
}

static void hal_powerkey_example(void)
{
    bool ret_bool;
    hal_keypad_status_t ret_state;

#ifdef HAL_SLEEP_MANAGER_ENABLED
    /*get sleep handle*/
    powerkey_lock_sleep_handle = hal_sleep_manager_set_sleep_handle(powerkey_lock_sleep_name);
    if (powerkey_lock_sleep_handle == INVALID_SLEEP_HANDLE) {
        log_hal_error("[pwk_main]:get sleep handle failed\r\n");
    }
#endif

    /*Initialize powerkey*/
    ret_bool = keypad_custom_powerkey_init();
    if (ret_bool == false) {
        log_hal_error("[pwk_main]keypad_custom_init init failed\r\n");
        return;
    }

    ret_state = hal_keypad_powerkey_register_callback((hal_keypad_callback_t)keypad_user_powerkey_handler, NULL);
    if (ret_state != HAL_KEYPAD_STATUS_OK) {
        log_hal_error("[pwk_main]hal_keypad_powerkey_register_callback failed, state = %d\r\n", ret_state);
    }
}
#endif

int mtk_sys_init(void)
{
#ifdef MTK_USB_DEMO_ENABLED
    usb_boot_init();
#endif

    vTaskStartScheduler();

    return 0;
}

int md_init(void)
{
    rt_enter_critical();
    
    /* MD init done here */
    md_init_phase_2();
    rt_exit_critical();
    /* End md_init_phase_2. Need to wait '+CPIN: READY' */

    return 0;
}

int mtk_components_init(void)
{
    log_init(syslog_config_save, syslog_config_load, syslog_control_blocks); /* Need to init after `md_init_phase_2` */

    tcpip_init(NULL, NULL);
    rt_kprintf("lwIP-%d.%d.%d initialized!\n", LWIP_VERSION_MAJOR, LWIP_VERSION_MINOR, LWIP_VERSION_REVISION);

#ifdef MTK_TCPIP_FOR_NB_MODULE_ENABLE
    nb_netif_init();
#endif

    mux_ap_init();
    ril_init();
    /* start up AP Bridge Proxy task */
    apb_proxy_init();

#ifdef MTK_COAP_SUPPORT
#ifdef MTK_COAP_AT_CMD_ENABLE
    nw_coap_init();
#endif
#endif

    tel_conn_mgr_init();
    //tel_conn_mgr_ut_init();

#ifdef MTK_FOTA_ENABLE
    fota_init();
#ifdef APB_PROXY_FOTA_CMD_ENABLE
    fota_register_event(apb_proxy_fota_event_ind);
    #ifdef MTK_CTIOT_SUPPORT
    if (fota_is_executed() == true) {
        ctiot_at_restore(lwm2m_restore_result_callback, NULL);
    }
    #endif
#endif
#endif /* MTK_FOTA_ENABLE */

    nidd_init();

#ifdef MTK_ATCI_APB_PROXY_NETWORK_ENABLE
    socket_atcmd_init_task();
    apb_upsm_init_task();
#ifdef MTK_LWM2M_SUPPORT
    lwm2m_atcmd_init_task();
#endif
#ifdef MTK_ONENET_SUPPORT
    onenet_at_init();
#endif
#ifdef MTK_ONENET_SUPPORT
    dm_at_init();
#endif
#ifdef MTK_TMO_CERT_SUPPORT
    tmo_at_init();
#endif
#ifdef MTK_CTM2M_SUPPORT
    ctm2m_at_task_init();
#endif
#endif /* MTK_ATCI_APB_PROXY_NETWORK_ENABLE */

    simat_proxy_init();
    //test_wrong_simat_command_from_AP();

#ifdef HAL_TIME_CHECK_ENABLED
    hal_time_check_enable();
#endif
#ifdef HAL_RTC_FEATURE_SW_TIMER
    rtc_sw_timer_isr();
#endif
#if defined(MTK_HTTPCLIENT_SSL_ENABLE) && defined(NB_USING_HTTPCLIENT)
    auto_register_init();
#endif

#ifdef MTK_GNSS_ENABLE
    gnss_demo_main();
#endif

#ifdef MTK_LWM2M_AT_CMD_ENABLE
#ifdef MTK_LWM2M_SUPPORT
    extern int nb_app_enter();
    /* 2625 lwm2m app start */
    nb_app_enter();
#endif
#ifdef MTK_LWM2M_CT_SUPPORT
    ctiot_lwm2m_client_init();
#endif
#ifdef MTK_CTIOT_SUPPORT
    ctiot_at_init();
#endif
#endif

#ifdef SENSOR_DEMO
    sensor_manager_init();
#endif

#ifdef HAL_KEYPAD_FEATURE_POWERKEY
    hal_powerkey_example();
#endif

    /* Start the scheduler. */
    SysInitStatus_Set();

#ifdef BSP_USING_SLEEP_MANAGER

    extern void vPortSetupTimerInterrupt(void);
    extern void check_poweron_restore_systick(void);

    rt_base_t level;
    level  = rt_hw_interrupt_disable();
    /* adjust systick by power-on mode (deep sleep or  deeper sleep) */
    check_poweron_restore_systick();
    vPortSetupTimerInterrupt();
    rt_hw_interrupt_enable(level);
#endif

#if defined(RT_USING_IDLE_HOOK) && defined(MTK_SYSTEM_HANG_CHECK_ENABLE)

#if ( configUSE_IDLE_HOOK == 1 )
    /* config wdt hook */
    extern void vApplicationIdleHook( void );
    rt_thread_idle_sethook(vApplicationIdleHook);
#endif /* configUSE_IDLE_HOOK == 1 */

#endif /* RT_USING_IDLE_HOOK && MTK_SYSTEM_HANG_CHECK_ENABLE */

    mtk_system_ready();
    rt_kprintf("mtk components init ok\n");

    return 0;
}

/* big_assert can be removed when every trace is coming in exception*/
void big_assert(void)
{
    for(uint8_t loopa = 0; loopa < 10; loopa++)
    {
        FrHslString (0x000A, "Assert! Assert! Assert! Assert! Assert! Assert! Assert! Assert!");
    }
    FrHslString (0x000A, "Final line!");
}
