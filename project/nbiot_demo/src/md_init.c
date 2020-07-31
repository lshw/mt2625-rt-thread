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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <sys_init.h>

#include "n1_hw_md_clock_manager.h"

/* device.h includes */
#include "mt2625.h"

/* hal includes */
#include "hal.h"
#include "hal_pmu.h"
#include "memory_attribute.h"

#if !defined (PSC_API_H)
#include <psc_api.h>
#endif

#if defined(USE_ULS)
#include "uls_api.h"

#include <frhsl.h>
#include <sys_trace.h>

#include "exception_handler.h"

#if defined(MTK_PORT_SERVICE_ENABLE)
#include "serial_port.h"
#endif
#include "kigcc_arm.h"
#include "memory_attribute.h"

extern void uls_init(void);
extern void UlsDeepSleepPrepare(PscSleepAction sleepAction, PscPowerState powerState);
extern int Emmi_dma_init(void);
extern void dump_uls_registers (void);
extern void dump_uls_block(void);
extern void dump_mem_pool_watermark(void);

void dump_fingerprint(void);

extern void big_assert(void);

#endif /*USE_ULS*/

/* Genie core dump support */
extern void wait_user_input_for_dump(void);

extern void SecHwInit(void);
extern void KiOsReset(void);

extern void log_sys_info(bool coredump);

static void modem_service_memory_callback_init(void)
{
    char trace_buffer[200];
    const char *expr, *file;
    int line;

    /* Genie needs to sync the exception */
#ifdef USE_KIPRINTF_AS_PRINTF
    wait_user_input_for_dump();
#endif
    trace_status_request(TRACE_STATUS_IDLE);
    /* Shut down RF power supply */
    pmu_shutdown_rfsys_supplies();

    FrHslString(0x000A, " ------------- Exception called ------------- ");

    exception_get_assert_expr(&expr, &file, &line);

    if (file)
    {
        snprintf(trace_buffer, 200, "\r\n assert failed: %s", expr);
        FrHslString (0x000A, trace_buffer);
        snprintf(trace_buffer, 200, "\r\n  file: %s", file);
        FrHslString (0x000A, trace_buffer);
        snprintf(trace_buffer, 200, "\r\n  line: %d\n\r", line);
        FrHslString (0x000A, trace_buffer);
    }

    if (global_assert_user_var_t.user_var_present)
    {
        snprintf(trace_buffer, 200, "\r\n  User variables: [0]=%#08X, [1]=%#08X, [2]=%#08X", global_assert_user_var_t.user_var[0], global_assert_user_var_t.user_var[1], global_assert_user_var_t.user_var[2]);
        FrHslString (0x000A, trace_buffer);
    }
}

static void modem_service_memory_callback_dump(void)
{

    dump_fingerprint();

    dump_mem_pool_watermark();

    big_assert();

    dump_uls_registers();

    dump_uls_block();

    /* Dump the remaining GKI trace to UART */
#ifdef GKI_TRACE_DUMP
    DumpLoggedSignals();
#endif

}

void modem_service_memory_dump_init (void)
{
    exception_config_type callback_config;
    callback_config.init_cb = modem_service_memory_callback_init;
    callback_config.dump_cb = modem_service_memory_callback_dump;
    exception_register_callbacks(&callback_config);
}


/***********************************************************
 *  md_init_phase_2
 *
 ************************************************************/


void md_init_phase_2(void)
{

    char trace_buff[15];

    // NOTE: FOLLOWING ACTIONS ARE HERE JUST FOR FPGA WAKEUP PURPOSES. NEED TO
    // BE REMOVED LATER

    // Force modem HW power on
    N1HwMdsysClockManagerForceModemPowerDomainOnForFpga();

    // Turn on all clocks
    N1MdsysClockManagerInit();

    uls_init();
#if !defined(NO_EMMI_INTERFACE)
#if !defined(MTK_PORT_SERVICE_ENABLE)
    Emmi_dma_init();
#endif
#endif

    SecHwInit();

    modem_service_memory_dump_init();

    /* Print out system info to HSL */
    log_sys_info(false);

    KiOsReset();
    snprintf(trace_buff, 15,"GKI started.");
    FrHslString (0x000A, trace_buff);

#ifdef FEA_TEMP_PSC_DEVELOPMENT
    psc_register_sleep_callback(PSC_CLIENT_ULS, (PscSleepCb)UlsDeepSleepPrepare );
    psc_reserve_active_lock(PSC_CLIENT_ULS);
#endif

    /* Do not enable DMA Irq before KiOsReset ! */
    NVIC_EnableIRQ(DMA_MCU_IRQn);
}


