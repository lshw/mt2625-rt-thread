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


#include "auto_register.h"
#include "queue.h"
#include "task.h"
#include "syslog.h"
#include "nvdm.h"
#include "ril.h"
#include "tel_conn_mgr_common.h"
#include "tel_conn_mgr_app_api.h"
#include "httpclient.h"
#include "base64.h"
#include "cJSON.h"
#include "apb_proxy.h"
#include "tel_conn_mgr_bearer_iprot.h"
#include "apb_proxy_nw_cmd_gprot.h"

//#define AUTO_REG_UT
log_create_module(auto_reg, PRINT_LEVEL_INFO);

static QueueHandle_t auto_reg_queue_handle;
static TaskHandle_t auto_reg_handle = NULL;
static auto_reg_context_t g_auto_reg_cntx;
static unsigned int cpin_ready = 0;

static int32_t auto_register_query_swver_callback(ril_cmd_response_t *rsp);
static void auto_register_active_bearer(void);
static void auto_reg_retry_timeout_callback(TimerHandle_t xTimer);
static void auto_register_parse_json_packet(char *packet, uint32_t packet_len);
static uint32_t auto_register_create_json_packet(char *register_content);
static int32_t auto_register_query_imsi_callback(ril_cmd_response_t *rsp);
static int32_t auto_register_query_imei_callback(ril_cmd_response_t *rsp);
static int32_t auto_register_query_iccid_callback(ril_cmd_response_t *rsp);
static void auto_register_send_packet(void);
static void auto_register_msg_handler(auto_reg_message_t *msg);
static void auto_register_task(void *arg);
static void auto_reg_task_free(void);
static void auto_register_read_iccid(void);
static int32_t auto_register_ril_urc_callback(ril_urc_id_t event_id, void *param, uint32_t param_len);
static void auto_register_prompt_message(uint32_t result);


static void auto_reg_retry_timeout_callback(TimerHandle_t xTimer)
{
    auto_reg_message_t *auto_reg_msg = NULL;
    
    AUTO_REG_LOGI("[auto_reg] timeout_callback!\n");
    auto_reg_msg = pvPortMalloc(sizeof(auto_reg_message_t));
    if (auto_reg_msg) {
        auto_reg_msg->message_id = MSG_ID_AUTO_REG_RETRY;
           
        if (pdTRUE != xQueueSend(auto_reg_queue_handle, &auto_reg_msg, 10)) {
           AUTO_REG_LOGI("[auto_reg] send msg fail!\n");
        }
    } else {
        AUTO_REG_LOGI("[auto_reg] msg = NULL\n");
    }
}

static void auto_reg_start_retry_timer(void)
{
    AUTO_REG_LOGI("[auto_reg] auto_reg_start_retry_timer, counter = %d\n", g_auto_reg_cntx.retry_counter);
    if (!g_auto_reg_cntx.retry_timer) {
            
        g_auto_reg_cntx.retry_timer = xTimerCreate("AUTO_REG_TIMER",
                                          AUTO_REG_RETRY_TIMER_PERIOD / portTICK_PERIOD_MS, 
                                          pdFALSE,
                                          ( void *)0,
                                          auto_reg_retry_timeout_callback);

        if (g_auto_reg_cntx.retry_timer == NULL) {
            AUTO_REG_LOGI("[auto_reg]Create retry timer fail!\r\n");
        }
    }

    if (g_auto_reg_cntx.retry_timer && g_auto_reg_cntx.retry_counter < AUTO_REG_RETRY_TIMES - 1) {
        g_auto_reg_cntx.retry_counter++;
        if (xTimerStart(g_auto_reg_cntx.retry_timer, 0) != pdPASS) {
            AUTO_REG_LOGI("[auto_reg]Retry timer start fail!\r\n");
        } 
    }
}

static void auto_reg_task_free(void)
{
    ril_status_t ret;
    
    if (auto_reg_handle != NULL) {
        vTaskDelete(auto_reg_handle);
        auto_reg_handle = NULL;
    }

    if (auto_reg_queue_handle)
    {
        vQueueDelete(auto_reg_queue_handle);
        auto_reg_queue_handle = NULL;
    }

    ret = ril_deregister_event_callback(auto_register_ril_urc_callback);
    if (ret != RIL_STATUS_SUCCESS) {
        AUTO_REG_LOGI("[auto_reg]Deregister RIL URC callback fail\r\n");
    }

    if (g_auto_reg_cntx.retry_timer) {
        xTimerStop(g_auto_reg_cntx.retry_timer, 0);
        g_auto_reg_cntx.retry_timer = NULL;
    }
    
    g_auto_reg_cntx.is_started = false;
}

static void auto_register_parse_json_packet(char *packet, uint32_t packet_len)
{
    char *pcode = packet + strlen("{\"resultCode\":\"");

    AUTO_REG_LOGI("[auto_reg]result code = %c\r\n", *pcode);

    if (*pcode == '0') {
        //register success, save the ICCID to NVRAM
        nvdm_write_data_item(AUTO_REG_NVDM_GROUP_NAME,
                           AUTO_REG_NVDM_DATA_ITEM_NAME,
                           NVDM_DATA_ITEM_TYPE_STRING,
                           (uint8_t *)g_auto_reg_cntx.iccid,
                           AUTO_REG_ICCID_LEN); 
        auto_register_prompt_message(1);
        auto_reg_task_free();
    } else {
        //start to retry
        auto_register_prompt_message(0);

        auto_reg_start_retry_timer();
        
    }
}


static uint32_t auto_register_create_json_packet(char *register_content)
{
    uint32_t content_len = 0;
    cJSON *root;
    char *out;
    
    root = cJSON_CreateObject();
    AUTO_REG_LOGI("[auto_reg]auto_register_create_json_packet\r\n");

    cJSON_AddStringToObject(root, "REGVER", AUTO_REG_REGVER_VALUE);   
    cJSON_AddStringToObject(root, "MEID", g_auto_reg_cntx.imei);
    cJSON_AddStringToObject(root, "MODELSMS", AUTO_REG_MODELSMS_VALUE);
    cJSON_AddStringToObject(root, "SWVER", g_auto_reg_cntx.swver);
    cJSON_AddStringToObject(root, "SIM1ICCID", g_auto_reg_cntx.iccid);    
    cJSON_AddStringToObject(root, "SIM1LTEIMSI", g_auto_reg_cntx.imsi);


    out = cJSON_Print(root); /* Print to text */
    cJSON_Delete(root);      /* Delete the cJSON object */
    content_len = strlen(out);
    AUTO_REG_LOGI("%s\r\n", out); /* Print out the text */
    
    AUTO_REG_LOGI("%d\r\n", content_len); /* Print out the text */
    memcpy(register_content, out, content_len);

    cJSON_free(out);	     /* Release the string. */

    return content_len;
}


static int32_t auto_register_query_swver_callback(ril_cmd_response_t *rsp)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;

    uint8_t swver_len = 0;
    ril_revision_identification_rsp_t *param = (ril_revision_identification_rsp_t *)rsp->cmd_param;

    if (rsp->res_code == RIL_RESULT_CODE_OK && param!= NULL){
        
        swver_len = strlen(param->revision);

        AUTO_REG_LOGI("[auto_reg]param->revision = %s,%d\r\n", param->revision, swver_len); 
        memcpy(g_auto_reg_cntx.swver, param->revision, (swver_len > AUTO_REG_SWVER_LEN)? AUTO_REG_SWVER_LEN : swver_len);
        //query IMSI
        
        ret = ril_request_imsi(RIL_ACTIVE_MODE, auto_register_query_imsi_callback, NULL);

        if (ret != RIL_STATUS_SUCCESS) {
            AUTO_REG_LOGI("[auto_reg]Query IMSI fail!\r\n");
        }
        
    } else {
        AUTO_REG_LOGI("[auto_reg]Query SWVER result fail!\r\n");

    }
    return 0;
}


static int32_t auto_register_query_imsi_callback(ril_cmd_response_t *rsp)
{
    uint8_t imsi_len = 0;
    auto_reg_message_t *auto_reg_msg = NULL;
    ril_imsi_rsp_t *param = (ril_imsi_rsp_t*)rsp->cmd_param;
    
    if (rsp->res_code == RIL_RESULT_CODE_OK && param!= NULL) {
        
        imsi_len = strlen(param->imsi);
        AUTO_REG_LOGI("[auto_reg]param->imsi = %s,%d\r\n", param->imsi, imsi_len);
        
        
        memcpy(g_auto_reg_cntx.imsi, param->imsi, (imsi_len > AUTO_REG_IMSI_LEN)? AUTO_REG_IMSI_LEN : imsi_len);
      

        //send message to AUTO REGISTER task

        auto_reg_msg = pvPortMalloc(sizeof(auto_reg_message_t));
        if (auto_reg_msg) {
            auto_reg_msg->message_id = MSG_ID_AUTO_REG_SIM_INFO_READY;
            
            if (pdTRUE != xQueueSend(auto_reg_queue_handle, &auto_reg_msg, 10)) {
                AUTO_REG_LOGI("[auto_reg] send msg fail!\r\n");
            }
        } else {
            AUTO_REG_LOGI("[auto_reg] msg = NULL\r\n");
        }
    } else {
        AUTO_REG_LOGI("[auto_reg]Query imsi result fail!\r\n");

    }
    return 0;
}


static int32_t auto_register_query_imei_callback(ril_cmd_response_t *rsp)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;

    uint8_t imei_len = 0;
    ril_serial_number_rsp_t *param = (ril_serial_number_rsp_t*)rsp->cmd_param;

    if (rsp->res_code == RIL_RESULT_CODE_OK && param!= NULL){
        
        imei_len = strlen(param->value.imei);

        AUTO_REG_LOGI("[auto_reg]param->value.imei = %s,%d\r\n", param->value.imei, imei_len);        
        memcpy(g_auto_reg_cntx.imei, param->value.imei, (imei_len > AUTO_REG_IMEI_LEN)? AUTO_REG_IMEI_LEN : imei_len);
        //query swver
        ret = ril_request_revision_identification(RIL_ACTIVE_MODE, auto_register_query_swver_callback, NULL);

        if (ret != RIL_STATUS_SUCCESS) {
            AUTO_REG_LOGI("[auto_reg]Query swver fail!\r\n");
        }
    } else {
        AUTO_REG_LOGI("[auto_reg]Query imei result fail!\r\n");

    }
    return 0;
}


static int32_t auto_register_query_iccid_callback(ril_cmd_response_t *rsp)
{
    nvdm_status_t status = NVDM_STATUS_OK;
    uint8_t saved_iccid[AUTO_REG_ICCID_LEN];
    uint32_t iccid_len;

    ril_read_usim_iccid_rsp_t *param = (ril_read_usim_iccid_rsp_t *)rsp->cmd_param;
    
    AUTO_REG_LOGI("[auto_reg]auto_register_query_iccid_callback\r\n");
    if (rsp->res_code == RIL_RESULT_CODE_OK && param != NULL) {
        
        AUTO_REG_LOGI("[auto_reg]param->iccid:%s!\r\n", param->iccid);
        iccid_len = strlen(param->iccid);
        memcpy(g_auto_reg_cntx.iccid, param->iccid, (iccid_len > AUTO_REG_ICCID_LEN)? AUTO_REG_ICCID_LEN : iccid_len);    

        //read ICCID from NVDM

        status = nvdm_read_data_item(AUTO_REG_NVDM_GROUP_NAME, AUTO_REG_NVDM_DATA_ITEM_NAME, saved_iccid, &iccid_len);

        if (status != NVDM_STATUS_OK || strncmp((const char *)saved_iccid, g_auto_reg_cntx.iccid, AUTO_REG_ICCID_LEN)) {

            AUTO_REG_LOGI("[auto_reg]ICCID is different\r\n");
            //need to auto register
            ril_status_t ret = RIL_STATUS_SUCCESS;
            
            ret = ril_request_serial_number(RIL_EXECUTE_MODE, 1, auto_register_query_imei_callback, NULL);

            if (ret != RIL_STATUS_SUCCESS) {
                AUTO_REG_LOGI("[auto_reg]Query IMEI fail!");
            }
        } else {
            AUTO_REG_LOGI("[auto_reg]ICCID is same, no need to auto-register!\r\n");
            //auto_register_prompt_message(2);
            auto_reg_task_free();
        }
    }

    return 0;
}


static void auto_register_send_packet(void)
{
    char register_content[AUTO_REG_CONTENT_LEN] = {0};
    uint32_t content_len;
    char  encoded_data[AUTO_REG_CONTENT_LEN] = {0};
    size_t encoded_len;
    int result;  
    content_len = auto_register_create_json_packet(register_content);

    AUTO_REG_LOGI("[auto_reg]content_len:%d\r\n", content_len);
    AUTO_REG_LOGI("%s\r\n", register_content);
    result = mbedtls_base64_encode((unsigned char *)encoded_data, AUTO_REG_CONTENT_LEN, &encoded_len, (const unsigned char *)register_content, content_len);
    //AUTO_REG_LOGI("%s\r\n", encoded_data);
  
    AUTO_REG_LOGI("[auto_reg]encodec_len = %d, result = %d\r\n", encoded_len, result);


#ifdef AUTO_REG_UT
{
    char decoded_data[AUTO_REG_CONTENT_LEN] = {0};
    size_t decoded_len;
    mbedtls_base64_decode(decoded_data, AUTO_REG_CONTENT_LEN, &decoded_len, encoded_data, encoded_len);
    AUTO_REG_LOGI("@3: %s\r\n", decoded_data);
}
#endif
    if (result == 0) {
        HTTPCLIENT_RESULT ret;
        httpclient_t auto_reg_client = {0};
        httpclient_data_t client_data;
        char *content_type = "application/encrypted-json";
        char *header = "x-forwarded-for:117.61.5.70\r\n";
        memset(&client_data, 0, sizeof(httpclient_data_t));

        auto_reg_client.header = header;
        auto_reg_client.timeout_in_sec = 60;//if http fail to receive, timeout
        
        client_data.response_buf = pvPortMalloc(AUTO_REG_RESPONSE_BUF_SIZE);
        client_data.response_buf_len = AUTO_REG_RESPONSE_BUF_SIZE;    
        client_data.response_content_len = -1;
        client_data.post_content_type = content_type;
        client_data.post_buf = encoded_data;
        client_data.post_buf_len = encoded_len;
        client_data.header_buf = pvPortMalloc(AUTO_REG_RESPONSE_BUF_SIZE);
        client_data.header_buf_len = AUTO_REG_RESPONSE_BUF_SIZE;   
        ret = httpclient_post(&auto_reg_client, AUTO_REG_POST_URL, &client_data);
        AUTO_REG_LOGI("[auto_reg]httpclient_post:%d!", ret);

        if (ret != HTTPCLIENT_OK) { 
            vPortFree(client_data.response_buf);            
            vPortFree(client_data.header_buf);
            auto_register_prompt_message(0);

            auto_reg_start_retry_timer();
            return;
        }
        AUTO_REG_LOGI("[auto_reg]len:%d, %s!", client_data.response_buf_len, client_data.response_buf);
         
        auto_register_parse_json_packet(client_data.response_buf, client_data.response_buf_len);

        vPortFree(client_data.response_buf);
        vPortFree(client_data.header_buf);
    } else {
        AUTO_REG_LOGI("[auto_reg]Base64 encode fail!");
    }
}

static void auto_register_prompt_message(uint32_t result)
{
    apb_proxy_at_cmd_result_t cmd_result;
    char data[128] = {0};
    apb_proxy_status_t stat;

    cmd_result.result_code = APB_PROXY_RESULT_UNSOLICITED;
    switch (result) {
        case 1: {
            strcpy(data, "CT-Self Register: Success!\r\n");
            break;
        }
        case 0: {
            strcpy(data, "CT-Self Register: Fail!\r\n");
            break;
        }
        default:
            break;
    }
    cmd_result.pdata = data;
    cmd_result.length = strlen((const char*)(cmd_result.pdata));
    
    cmd_result.cmd_id = APB_PROXY_INVALID_CMD_ID;
    stat = apb_proxy_send_at_cmd_result(&cmd_result);      

    AUTO_REG_LOGI("[auto_reg] string length:%d, result = %d, prompt_state = %d\r\n", cmd_result.length, result, stat);
}



static void auto_register_active_bearer(void)
{
    tel_conn_mgr_pdp_type_enum activated_pdp_type = TEL_CONN_MGR_PDP_TYPE_NONE;
    tel_conn_mgr_ret_enum ret;    
 
    //step1 check oprator

    AUTO_REG_LOGI("[auto_reg]imsi:%s!\r\n", g_auto_reg_cntx.imsi);

    if (!strncmp(g_auto_reg_cntx.imsi, "46011", 5) || !strncmp(g_auto_reg_cntx.imsi, "46003", 5) || !strncmp(g_auto_reg_cntx.imsi, "46005", 5)) {
            
         ip_addr_t ns;

         ip_addr_set_ip4_u32(&ns, ipaddr_addr("218.4.4.4"));
         dns_setserver(0, &ns);
        //step2 activate the bearer
        ret = tel_conn_mgr_activate(TEL_CONN_MGR_BEARER_TYPE_NBIOT,
            TEL_CONN_MGR_SIM_ID_1,
            TEL_CONN_MGR_PDP_TYPE_IPV4V6,
            "",
            "",
            "",
            auto_reg_queue_handle,
            &g_auto_reg_cntx.app_id,
            &activated_pdp_type);
        AUTO_REG_LOGI("[auto_reg]tel_conn_mgr_activate:%d!\r\n", ret);


        if (TEL_CONN_MGR_RET_OK == ret) {
            
            auto_register_send_packet();

            tel_conn_mgr_deactivate(g_auto_reg_cntx.app_id);
        } else {//TEL_CONN_MGR_RET_WOULDBLOCK, need listen msg.
            return;
        }    
    
    } else {

        AUTO_REG_LOGI("[auto_reg]APN is not CT!\r\n");
        auto_reg_task_free();
    }
}

static void auto_register_msg_handler(auto_reg_message_t *msg)
{
    
    AUTO_REG_LOGI("[auto_reg]msg_handler:%d!\r\n", msg->message_id);
    switch(msg->message_id) {
        case MSG_ID_TEL_CONN_MGR_ACTIVATION_RSP://513//MSG_ID_TEL_CONN_MGR_ACTIVATION_RSP:
        {
            tel_conn_mgr_activation_rsp_struct *active_msg = (tel_conn_mgr_activation_rsp_struct *)msg;

            AUTO_REG_LOGI("[auto_reg]active result = %d\r\n", active_msg->result);
            if (active_msg->result) {
                auto_register_send_packet();
                tel_conn_mgr_deactivate(g_auto_reg_cntx.app_id);
            } else {
                //retry
                auto_reg_start_retry_timer();
            }
        }
            break;

        case MSG_ID_TEL_CONN_MGR_DEACTIVATION_RSP://514
            break;

        case MSG_ID_TEL_CONN_MGR_DEACTIVATION_IND://515
            break;

        case MSG_ID_AUTO_REG_SIM_INFO_READY://336
        {      
            AUTO_REG_LOGI("[auto_reg]MSG_ID_AUTO_REG_SIM_INFO_READY received\r\n");

            auto_register_active_bearer();
        }
            break;
        case MSG_ID_AUTO_REG_START://337
        {
        #if defined(MTK_ONENET_SUPPORT) && defined(MTK_ONENET_AT_CMD_SUPPORT)
            dm_at_register();
        #endif
            auto_register_read_iccid();
        }
            break;
        case MSG_ID_AUTO_REG_RETRY://338
        {
            auto_register_active_bearer();

        }
            break;
               
        default:
            break;
            
    }
    
    vPortFree(msg);
}


static void auto_register_task(void *arg)
{
    auto_reg_message_t *queue_item = NULL;
    while (1) {
        if (xQueueReceive(auto_reg_queue_handle, &queue_item, portMAX_DELAY)) {
            auto_register_msg_handler(queue_item);
        }
    }
}
static void auto_register_read_iccid(void)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    ret = ril_request_read_usim_iccid(RIL_ACTIVE_MODE, auto_register_query_iccid_callback, NULL);
    AUTO_REG_LOGI("[auto_reg]auto_register_read_iccid,ret:%d\r\n", ret);

    if (ret != RIL_STATUS_SUCCESS) {
        AUTO_REG_LOGI("[auto_reg]Query iccid fail!");
    }

}


static int32_t auto_register_ril_urc_callback(ril_urc_id_t event_id, void *param, uint32_t param_len)
{  

    switch(event_id)
    {
        case RIL_URC_ID_CPIN: {  //10
            ril_enter_pin_urc_t * pin = (ril_enter_pin_urc_t*)param;

            if (strcmp(pin->code, "READY") == 0) {
                if (cpin_ready == 0) {
                    int32_t ret = 0;
                    
                    cpin_ready = 1;
                    ret = ril_request_eps_network_registration_status(RIL_EXECUTE_MODE,
                                                                     2,
                                                                     NULL,
                                                                     NULL);
                }

           }
           break;
       }

        case RIL_URC_TYPE_NW_PDN_ACT:
        {
            auto_reg_message_t *auto_reg_msg = NULL;
            
            AUTO_REG_LOGI("[auto_reg] RIL_URC_TYPE_NW_PDN_ACT\n");

            auto_reg_msg = pvPortMalloc(sizeof(auto_reg_message_t));
            if (auto_reg_msg) {
                auto_reg_msg->message_id = MSG_ID_AUTO_REG_START;
                if (auto_reg_queue_handle && g_auto_reg_cntx.is_started == false) {
                    if (pdTRUE != xQueueSend(auto_reg_queue_handle, &auto_reg_msg, 10)) {
                        AUTO_REG_LOGI("[auto_reg] send msg fail!\n");
                    } else {
                        g_auto_reg_cntx.is_started = true;
                    }
                } else {
                    vPortFree(auto_reg_msg);
                }
            } else {
                AUTO_REG_LOGI("[auto_reg] msg = NULL!\n");
            }
        }
            break;
            
        case RIL_URC_ID_CEREG:
        {
            /*ril_eps_network_registration_status_urc_t *cereg_status = (ril_eps_network_registration_status_urc_t *)param;

            AUTO_REG_LOGI("[auto_reg] RIL_URC_ID_CEREG, state = %d\n", cereg_status->stat);

            if (TEL_CONN_MGR_NW_REG_STAT_REGED_HMNW == cereg_status->stat ||
                TEL_CONN_MGR_NW_REG_STAT_REGED_ROAMING == cereg_status->stat ||
                TEL_CONN_MGR_NW_REG_STAT_SMS_ONLY_HMNW == cereg_status->stat ||
                TEL_CONN_MGR_NW_REG_STAT_SMS_ONLY_ROAMING == cereg_status->stat) {//1, 5, 6, 7
                //register to the network successfully
                auto_reg_message_t *auto_reg_msg = NULL;

                auto_reg_msg = pvPortMalloc(sizeof(auto_reg_message_t));
                if (auto_reg_msg) {
                    auto_reg_msg->message_id = MSG_ID_AUTO_REG_START;
                    if (auto_reg_queue_handle && g_auto_reg_cntx.is_started == false) {
                        if (pdTRUE != xQueueSend(auto_reg_queue_handle, &auto_reg_msg, 10)) {
                            AUTO_REG_LOGI("[auto_reg] send msg fail!\n");
                        } else {
                            g_auto_reg_cntx.is_started = true;
                        }
                    } else {
                        vPortFree(auto_reg_msg);
                    }
                } else {
                    AUTO_REG_LOGI("[auto_reg] msg = NULL!\n");
                }
            }*/
        }
            break;
        default:
            break;

    }
    
    
   return 0;
}


void auto_register_init(void)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;

    AUTO_REG_LOGI("[auto_reg]init\r\n");
    memset(&g_auto_reg_cntx,0,sizeof(auto_reg_context_t));
    auto_reg_queue_handle = xQueueCreate(AUTO_REG_TASK_QUEUE_SIZE, sizeof(auto_reg_message_t*));
    if( auto_reg_queue_handle == NULL ) {
        AUTO_REG_LOGI("[auto_reg]failed to create queue!\r\n");
        return;
    }
    
    if (pdPASS != xTaskCreate(auto_register_task, 
                AUTO_REG_TASK_NAME, 
                AUTO_REG_TASK_STACK_SIZE /((uint32_t)sizeof(portSTACK_TYPE)), 
                NULL, AUTO_REG_TASK_PRIORITY, 
                &auto_reg_handle)) {
        AUTO_REG_LOGI("[auto_reg]Create task fail!");
        return;
    }
    
    ret = ril_register_event_callback(RIL_GROUP_MASK_ALL, auto_register_ril_urc_callback);
    if (ret != RIL_STATUS_SUCCESS){
        AUTO_REG_LOGI("[auto_reg]register URC into RIL fail!");

    }
    
}
