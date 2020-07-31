/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
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

#ifndef __AUTO_REG_H__
#define __AUTO_REG_H__

#include "system.h"
#include "FreeRTOS.h"
#include "timers.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !defined (MTK_DEBUG_LEVEL_NONE)
#define AUTO_REG_LOGI(fmt, args...)   LOG_I(auto_reg, fmt, ##args)
#define AUTO_REG_LOGW(fmt, args...)   LOG_W(auto_reg, "[AUTO_REG] "fmt, ##args)
#define AUTO_REG_LOGE(fmt, args...)   LOG_E(auto_reg, "[AUTO_REG] "fmt, ##args)
#else
#define AUTO_REG_LOGI(fmt, args...)   printf("[AUTO_REG] "fmt"\r\n", ##args)
#define AUTO_REG_LOGW(fmt, args...)   printf("[AUTO_REG] "fmt"\r\n", ##args)
#define AUTO_REG_LOGE(fmt, args...)   printf("[AUTO_REG] "fmt"\r\n", ##args)
#endif

/********************************customer parameters********************************/
#define AUTO_REG_REGVER_VALUE           "2"
#define AUTO_REG_MODELSMS_VALUE         "MTK-MT2625"
#define AUTO_REG_POST_URL               "http://zzhc.vnet.cn:9999/"//"http://42.99.2.15:9999/"
/********************************customer parameters********************************/


#define AUTO_REG_NVDM_GROUP_NAME        "auto_reg"
#define AUTO_REG_NVDM_DATA_ITEM_NAME    "iccid"
#define AUTO_REG_ICCID_LEN              20
#define AUTO_REG_SWVER_LEN              20

#define AUTO_REG_TASK_QUEUE_SIZE        10
#define AUTO_REG_TASK_NAME              "auto_register"
#define AUTO_REG_TASK_STACK_SIZE        (4*1024)
#define AUTO_REG_TASK_PRIORITY          TASK_PRIORITY_NORMAL
#define AUTO_REG_CONTENT_LEN            512

#define AUTO_REG_IMEI_LEN               15
#define AUTO_REG_IMSI_LEN               15


#define AUTO_REG_RETRY_TIMER_PERIOD     3600000//10000
#define AUTO_REG_RETRY_TIMES            3

#define AUTO_REG_RESPONSE_BUF_SIZE      512
#define MSG_ID_AUTO_REG_BASE            (0x150)
#define MSG_ID_AUTO_REG_SIM_INFO_READY  (MSG_ID_AUTO_REG_BASE)
#define MSG_ID_AUTO_REG_START           (MSG_ID_AUTO_REG_BASE+1)
#define MSG_ID_AUTO_REG_RETRY           (MSG_ID_AUTO_REG_BASE+2)

typedef struct {
    bool is_started;

    unsigned int app_id;
    TimerHandle_t retry_timer;
    uint8_t retry_counter;

    char imei[AUTO_REG_IMEI_LEN + 1];
    char imsi[AUTO_REG_IMSI_LEN + 1];
    char iccid[AUTO_REG_ICCID_LEN + 1];
    char swver[AUTO_REG_SWVER_LEN + 1];
} auto_reg_context_t;


typedef struct {
    unsigned int message_id;
    void *param;
} auto_reg_message_t;


void auto_register_init(void);
#ifdef __cplusplus
}
#endif

#endif

