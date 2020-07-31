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

#ifndef APP_COMMON_NB_EXAMPLE_HEADER
#define APP_COMMON_NB_EXAMPLE_HEADER


#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "stdio.h"
#include "stdlib.h"

#include "liblwm2m.h"
#include "ril.h"


#define MAX_DATA_BUFFER_NUM                                 (1)
#define DATA_COLLECT_PERIOD                                 (8000)

#define msg_data_collect_complete                           (100)
#define msg_app_state_change                                (101)
#define msg_app_send_complete                               (102)


#define MAX_APN_SIZE                                        (50)
#define MAX_USERNAME_SIZE                                   (20)
#define MAX_PASSWD_SIZE                                     (20)
#define PDP_VALUE_SIZE                                      (1)

#define MAX_SERVER_URL_LEN                                  (50)
#define MAX_BIND_TYUPE_LEN                                  (2)

#define LWM2M_CLIENT_NAME_LENGTH							(32)

#define MAX_LOCATION_LENGTH 								(20)      // strlen("/rd/65534") + 1

#define ACCELEROMETER_TOTAL_LEN_UNITS                      (15)
#define total_acceleromter_len                             (ACCELEROMETER_TOTAL_LEN_UNITS * 3)

#define LWM2M_SENSOR_SRV_ID                                (123)

#define LWM2M_FLAG_CFUN_READY                        (1 << 0)
#define LWM2M_FLAG_CLIENT_RUNNING                    (1 << 1)
#define LWM2M_FLAG_CLIENT_RESTART                    (1 << 2)
#define LWM2M_FLAG_CEREG_ACTVIE                      (1 << 3)

#define LWM2M_SET_FLAG(FLAG, MASK) do { \
    (FLAG) |= (MASK); \
} while(0);

#define LWM2M_RESET_FLAG(FLAG, MASK) do { \
    (FLAG) &= ~(MASK); \
} while(0);


#define LWM2M_MSG_ID_BASE                          (0x600000)
#define LWM2M_MSG_ID_CLIENT_RESTART                (LWM2M_MSG_ID_BASE + 1)
#define LWM2M_MSG_ID_CEREG_ACTIVE                  (LWM2M_MSG_ID_BASE + 2)
#define LWM2M_MSG_ID_UPDATE_REGISTER               (LWM2M_MSG_ID_BASE + 3)
#define LWM2M_MSG_ID_LWM2M_INIT                    (LWM2M_MSG_ID_BASE + 4)


#define LWM2M_BEARER_TRY_MAX_TIME                  (10)

#define LWM2M_CLI_PORT                             (61520)

typedef SemaphoreHandle_t lwm2m_app_mutex_t;

typedef void (*upload_cb_ptr)(void);
typedef void (*collect_data_complete_cb_ptr)(void);
typedef void (*collect_data_complete_cb)(void);
typedef void (*nb_data_collect_rtos_expired_t)(TimerHandle_t xTimer);


typedef struct {
    unsigned char apn_val[MAX_APN_SIZE];
    unsigned char user_val[MAX_USERNAME_SIZE];
    unsigned char passwd_val[MAX_PASSWD_SIZE];
    unsigned char pdp_val;
    unsigned int  valid_flag;
    unsigned int  app_id;
}simat_bearer_info;



typedef enum NB_APP_STATE{
    APP_INIT = 0,
    APP_IDLE,
    APP_DATA_COLLECTING,
    APP_DATA_REPORTING,
    APP_DATA_RECVING
}NB_APP_STATE;


typedef struct barometer_sensor_data
{
    double    value;     /* R */
}barometer_sensor_data;


typedef struct accelerometer_sensor_data
{
    double    value_x;     /* R */
    double    value_y;     /* R */
    double    value_z;     /* R */
} accelerometer_sensor_data;


typedef struct nb_accelerometer_data_blocks{
    accelerometer_sensor_data sensor_accelerometer[MAX_DATA_BUFFER_NUM];
    int current_use_num;
    int total_num;
} nb_accelerometer_data_blocks;


typedef struct nb_barometer_data_blocks{
    barometer_sensor_data sensor_barometer[MAX_DATA_BUFFER_NUM];
    int current_use_num;
    int total_num;
}nb_barometer_data_blocks;


typedef struct nb_app_bearer_info{
    simat_bearer_info   app_simat_info;                 /* app simat info */
    lwm2m_app_mutex_t   app_bearer_semaphore;
    bool                app_bearer_valid_flag;          /* bearer is connected or not */
}nb_app_bearer_info;


typedef struct nb_app_context{  
    NB_APP_STATE app_state;                             /* app logical status */
    nb_app_bearer_info bearer_info;                     /* bearer related data */
    collect_data_complete_cb_ptr data_result_cb;
    lwm2m_context_t * lwm2m_info;
    int data_collect_period;
    int force_upload_period;
    bool is_deep_sleep;
    bool not_send_bearer_flag;
    nb_accelerometer_data_blocks accelerometer_blocks;  /* nb barometer sensor data cache store */
    nb_barometer_data_blocks barometer_blocks;
    uint16_t bearer_try_time;
    uint16_t cli_port;
#ifdef MTK_LWM2M_CT_SUPPORT
    bool activated_from_command;
    bool wait_for_deleted;
    char *server;
    char *port;
    char *name;
    char *pskid;
    char *psk;
#endif
    int lifetime;
}nb_app_context;


typedef struct lwm2m_app_object_data{
    /* device name */
    //char deice_name[LWM2M_CLIENT_NAME_LENGTH];
    int tickCount; /* Construct sensor device name */
    /* security object */
    //char server_uri[MAX_SERVER_URL_LEN];
    int server_id;
    /* server project */
    //char bind_type[MAX_BIND_TYUPE_LEN];
    int life_time;
    /* location */
    //char location[MAX_LOCATION_LENGTH];
    /* lwm2m status */
    lwm2m_client_state_t state;
    /* app status */
    NB_APP_STATE app_state;

    /* sensor data */
    accelerometer_sensor_data sensor_accelerometer[MAX_DATA_BUFFER_NUM];
    uint16_t used_number;

    /* Update register */
    uint16_t hr_count; 
}lwm2m_app_object_data;


extern lwm2m_app_object_data g_collect_data;

extern uint32_t g_nb_lwm2m_flag;

void barometer_data_collect(double value);


void accelerometer_data_collect(double value_x, 
                                     double value_y, 
                                     double value_z);
void app_send_msg(unsigned int msg_id, void *param);


void handle_value_changed(lwm2m_context_t * lwm2mH,
                          lwm2m_uri_t * uri,
                          const char * value,
                          size_t valueLength);

char* get_accelerometer_uri_string(char type);

char* get_barometer_uri_string();


void lwm2m_app_mutex_take(void);

/* data rentation API */
void save_security_object(char *bind_type, int server_id);
void save_server_object(char *bind_type, int life_time);
void save_client_name(char *client_name);

void nb_app_notify_lwm2m_client_restart(void);

int32_t nb_app_is_nw_registered(ril_eps_network_registration_status_urc_t *nw_reg_status);

#endif /* APP_COMMON_NB_EXAMPLE_HEADER */
