/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-24     MurphyZhao   First version
 */

/**
 * Encoding format: UTF-8
 * 
 * 参考 ril.h
 * 
 * 每次上电查一次的参数
 * 
 * - [Y] AT+CGMI   制造商识别
 * - [Y] AT+CGMM   请求模组 ID
 * - [Y] AT+CGMR   请求修订标识
 * - [Y] AT+CGSN   请求产品序列号
 * - [Y] AT+CIMI   查 sim 卡 IMSI
 * - [Y] AT+GSN    查 IMEI
 * - [Y] AT*MICCID 查 USIM ICCID
 * 
 * 需要独立查询的参数
 * 
 * - [Y] AT+CEREG  查询网络注册状态
 * - [N] AT+CGACT  查驻网状态，查询网络是否注册
 * - [Y] AT+CSQ    查信号质量（RSSI 信号强度）
 * - [Y] AT+CEDRXS eDRX 设置
 * - [N] AT+CEDRXRDP 读取 eDRX 动态参数
 * - [Y] AT+CPSMS  控制 UE 的 PSM 模式，设置 TAU 值和活动时间
 * 
 * 控制类
 * 
 * - [Y] AT+CFUN
 * 
*/

#include "stdint.h"
#include <rtthread.h>

#include "rt_nb_port.h"
#include "ril.h"

#ifdef RT_NB_DBG_FEATURE
#define LOG_TAG              "ril.port"
#define LOG_LVL              LOG_LVL_ERROR // LOG_LVL_ERROR, LOG_LVL_DBG
#include <ulog.h>
#else
#define LOG_D(...) 
#define LOG_I(...) 
#define LOG_W(...) 
#define LOG_E(...) 
#endif

/* Blocking waiting for ril_request_xxx result */
static rt_sem_t local_nb_ril_sync_sem = RT_NULL;

/* Mutually exclusive operation for ril_request_xxx */
static rt_mutex_t local_nb_ril_mutex = RT_NULL;

static rt_nb_cfg_t local_nb_cfg;

static uint8_t local_nb_ril_inited = 0;
static int32_t local_nb_rssi = 99, local_nb_ber = 99;
static int32_t local_ril_cmd_result = RT_NB_RIL_CMD_OK;

static int32_t _get_serial_num_cb(ril_cmd_response_t *rsp)
{
    uint8_t sn_type;
    ril_serial_number_rsp_t *param;

    if (!rsp || (rsp->res_code != RIL_RESULT_CODE_OK) || (!rsp->cmd_param))
    {
        if (rsp)
        {
            LOG_D("ril request failed, ret:%d", rsp->res_code);
        }
        else
        {
            LOG_D("ril request failed");
        }
        goto __exit;
    }

    param = (ril_serial_number_rsp_t*)rsp->cmd_param;
    sn_type = (uint8_t)((uint32_t)rsp->user_data);
    LOG_D("cmd id:%d; req mod:%d", rsp->cmd_id, rsp->mode);

    if (sn_type == 0)
        rt_strncpy(local_nb_cfg.sn, param->value.sn, sizeof(local_nb_cfg.sn));
    else if (sn_type == 1)
        rt_strncpy(local_nb_cfg.imei, param->value.imei, sizeof(local_nb_cfg.imei));
    else if (sn_type == 2)
        rt_strncpy(local_nb_cfg.imeisv, param->value.imeisv, sizeof(local_nb_cfg.imeisv));
    else if (sn_type == 3)
        rt_strncpy(local_nb_cfg.svn, param->value.svn, sizeof(local_nb_cfg.svn));

__exit:
    rt_sem_release(local_nb_ril_sync_sem);
    return 0;
}

static int32_t _ri_request_common_cb(ril_cmd_response_t *rsp)
{
    local_ril_cmd_result = RT_NB_RIL_CMD_OK;

    if (!rsp || (rsp->res_code != RIL_RESULT_CODE_OK))
    {
        if (rsp)
        {
            LOG_D("ril request failed, ret:%d", rsp->res_code);
        }
        else
        {
            LOG_D("ril request failed");
        }
        local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        goto __exit;
    }
    LOG_D("cmd id:%d; req mod:%d", rsp->cmd_id, rsp->mode);

    switch (rsp->cmd_id)
    {
    case RIL_CMD_ID_CIMI:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_imsi_rsp_t *imsi_param = (ril_imsi_rsp_t*)rsp->cmd_param;
            if (imsi_param->imsi)
            {
                rt_strncpy(local_nb_cfg.imsi, imsi_param->imsi, sizeof(local_nb_cfg.imsi));
            }
            else
            {
                local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
            }
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_MICCID:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_read_usim_iccid_rsp_t *iccid_param = (ril_read_usim_iccid_rsp_t *)rsp->cmd_param;
            if (iccid_param->iccid)
            {
                rt_strncpy(local_nb_cfg.iccid, iccid_param->iccid, sizeof(local_nb_cfg.iccid));
            }
            else
            {
                local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
            }
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_CSQ:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_signal_quality_rsp_t *param = (ril_signal_quality_rsp_t *)rsp->cmd_param;
            local_nb_rssi = param->rssi;
            local_nb_ber  = param->ber;
            LOG_D("rssi: %d", (int) param->rssi);
            LOG_D("ber : %d", (int) param->ber);
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_CESQ:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_extended_signal_quality_rsp_t *param = 
                    (ril_extended_signal_quality_rsp_t *)rsp->cmd_param;
            if (rsp->user_data)
            {
                rt_memcpy(rsp->user_data, param, sizeof(ril_extended_signal_quality_rsp_t));
            }
            else
            {
                local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
            }
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_CEREG:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_eps_network_registration_status_rsp_t *param = 
                    (ril_eps_network_registration_status_rsp_t *)rsp->cmd_param;
            if (rsp->user_data)
            {
                rt_memcpy(rsp->user_data, 
                        param, sizeof(ril_eps_network_registration_status_rsp_t));
            }
            else
            {
                local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
            }
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_CFUN:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_set_phone_functionality_rsp_t *param = (ril_set_phone_functionality_rsp_t *)rsp->cmd_param;
            if (rsp->user_data)
            {
                ril_set_phone_functionality_rsp_t *cfun_rsp = (ril_set_phone_functionality_rsp_t *)(rsp->user_data);
                cfun_rsp->fun = param->fun;
            }
            else
            {
                local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
            }
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_CPIN:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_enter_pin_rsp_t *param = (ril_enter_pin_rsp_t *)rsp->cmd_param;
            if (rsp->user_data)
            {
                uint8_t *status = (uint8_t *)(rsp->user_data);
                if (!strcmp(param->code, "READY"))
                {
                    *status = 1;
                }
                else
                {
                    *status = 0;
                }
            }
            else
            {
                local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
            }
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_CGDCONT:
    {
        if (rsp->cmd_param != NULL)
        {
            ril_define_pdp_context_rsp_t *param = 
                    (ril_define_pdp_context_rsp_t*)rsp->cmd_param;
            if (param->array_num > 0 && rsp->user_data) {
                memcpy(rsp->user_data, param->pdp_context[0].apn, RT_NB_MAX_APN_NAME);
            }
            else
            {
                local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
            }
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_MCGDEFCONT:
    {
        LOG_D("set apn ret:%d", rsp->res_code);
        break;
    }
    case RIL_CMD_ID_CEDRXS:
    {
        break;
    }
    case RIL_CMD_ID_CPSMS:
    {
        break;
    }
    case RIL_CMD_ID_CGPADDR:
    {
        if (rsp->cmd_param != NULL && rsp->res_code == RIL_RESULT_CODE_OK && rsp->user_data)
        {
            ril_show_pdp_address_rsp_t *param = 
                    (ril_show_pdp_address_rsp_t*)rsp->cmd_param;

            for (int idx = 0; idx < param->array_num; idx++)
            {
                if(1 == param->cid_addr[idx].cid)
                {
                    memcpy(rsp->user_data, param->cid_addr[idx].pdp_addr_1, 
                            strlen(param->cid_addr[idx].pdp_addr_1));
                    local_ril_cmd_result = RT_NB_RIL_CMD_OK;
                    goto __exit;
                }
            }
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    case RIL_CMD_ID_CCLK:
    {
        if (rsp->cmd_param != NULL && rsp->res_code == RIL_RESULT_CODE_OK && rsp->user_data)
        {
            ril_clock_rsp_t *param = (ril_clock_rsp_t *)rsp->cmd_param;
            rt_ext_bytes_t *buf = (rt_ext_bytes_t *)rsp->user_data;
            rt_strncpy(buf->buf, param->time, buf->len);
            local_ril_cmd_result = RT_NB_RIL_CMD_OK;
        }
        else
        {
            local_ril_cmd_result = RT_NB_RIL_CMD_GENERAL_ERROR;
        }
        break;
    }
    default:
        break;
    }

__exit:
    rt_sem_release(local_nb_ril_sync_sem);
    return 0;
}

rt_err_t rt_nb_ril_init(void)
{
    if (local_nb_ril_inited == 1)
    {
        return RT_EOK;
    }

    local_nb_ril_sync_sem = rt_sem_create("ril_sync", 0, RT_IPC_FLAG_FIFO);
    if (local_nb_ril_sync_sem == RT_NULL)
    {
        LOG_E("ril_sync sem create failed");
        return -RT_ERROR;
    }

    local_nb_ril_mutex = rt_mutex_create("ril_mux", RT_IPC_FLAG_FIFO);
    if (local_nb_ril_mutex == RT_NULL)
    {
        LOG_E("ril_mutex create failed");
        return -RT_ERROR;
    }
    local_nb_ril_inited = 1;
    return RT_EOK;
}

/**
 * SN, IMEI
 * AT+CGSN
*/
int rt_nb_cfg_sync(void)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;

    if (local_nb_ril_inited != 1)
    {
        if (rt_nb_ril_init() != RT_EOK)
        {
            return -1;
        }
    }

    /* query cpin status */
    {
        uint8_t sim_card_status = 0;

        while(1)
        {
            rt_nb_sim_card_status_get(&sim_card_status);
            if (sim_card_status != 0)
            {
                break;
            }

            rt_thread_mdelay(1000);
        }
        LOG_D("sim card ready");
    }

    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    /* IMEI */
    ret = ril_request_serial_number(RIL_EXECUTE_MODE, 1, _get_serial_num_cb, (void *)1);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset IMEI failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    /* SN */
    ret = ril_request_serial_number(RIL_EXECUTE_MODE, 0, _get_serial_num_cb, (void *)0);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset SN failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    /* IMEISV */
    ret = ril_request_serial_number(RIL_EXECUTE_MODE, 2, _get_serial_num_cb, (void *)2);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset IMEISV failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    /* SVN */
    ret = ril_request_serial_number(RIL_EXECUTE_MODE, 3, _get_serial_num_cb, (void*)3);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset svn failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    /* IMSI */
    ret = ril_request_imsi(RIL_ACTIVE_MODE, _ri_request_common_cb, NULL);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset IMSI failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    /* ICCID */
    ret = ril_request_read_usim_iccid(RIL_ACTIVE_MODE, _ri_request_common_cb, NULL);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset IMSI failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret;
}

rt_nb_cfg_t *rt_nb_cfg_get(void)
{
    return (rt_nb_cfg_t *)&local_nb_cfg;
}

char *rt_nb_imei_get(void)
{
    return (char *)local_nb_cfg.imei;
}

char *rt_nb_imsi_get(void)
{
    return (char *)local_nb_cfg.imsi;
}

char *rt_nb_iccid_get(void)
{
    return (char *)local_nb_cfg.iccid;
}

char *rt_nb_sn_get(void)
{
    return (char *)local_nb_cfg.sn;
}

/* CSQ */
rt_err_t rt_nb_csq_get(int32_t *rssi, int32_t *ber)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    if (!rssi || !ber)
    {
        return -RT_ERROR;
    }

    if (local_nb_ril_inited != 1)
    {
        return -RT_ERROR;
    }

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_signal_quality(RIL_ACTIVE_MODE, _ri_request_common_cb, RT_NULL);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset IMEI failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);

        *rssi = local_nb_rssi;
        *ber  = local_nb_ber;
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

/**
 * CESQ
*/
rt_err_t rt_nb_cesq_get(rt_nb_signal_quality_rsp_t *signal)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    ril_extended_signal_quality_rsp_t rsp;
    if (!signal)
    {
        return -RT_ERROR;
    }

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_extended_signal_quality(RIL_ACTIVE_MODE, 
                    _ri_request_common_cb, &rsp);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset IMEI failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);

        signal->rxlev = rsp.rxlev;
        signal->ber = rsp.ber;
        signal->rscp = rsp.rscp;
        signal->ecno = rsp.ecno;
        signal->rsrq = rsp.rsrq;
        signal->rsrp = rsp.rsrp;

        LOG_D("rxlev :%d", signal->rxlev);
        LOG_D("ber   :%d", signal->ber);
        LOG_D("rscp  :%d", signal->rscp);
        LOG_D("ecno  :%d", signal->ecno);
        LOG_D("rsrq  :%d", signal->rsrq);
        LOG_D("rsrp  :%d", signal->rsrp);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

/**
 * @param direction
 *          0: get
 *          1: set
*/
rt_err_t rt_nb_cfun_set_get(int32_t *status, uint8_t direction)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    ril_set_phone_functionality_rsp_t rsp = {0};
    if (!status)
    {
        LOG_E("In param status is null!");
        return -RT_ERROR;
    }

    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    if (direction == 1) /* set */
    {
        ret = ril_request_set_phone_functionality(RIL_EXECUTE_MODE, 
                        *status, 0, _ri_request_common_cb, &rsp);
    }
    else if (direction == 0) /* get */
    {
        ret = ril_request_set_phone_functionality(RIL_READ_MODE, 0, 0, 
                        _ri_request_common_cb, &rsp);
    }

    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset IMEI failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);

        if (direction == 0) /* get */
        {
            *status = rsp.fun;
        }
    }

    rt_mutex_release(local_nb_ril_mutex);

    LOG_D( "cfun, ret:%d; rsp:%d", ret, rsp.fun);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

/**
 * rt_nb_sim_card_status_get
 * 
 * @brief check sim card status
 * @param *status
 *          1: ready
 *          0: not ready
 * @return rt_err_t type
*/
rt_err_t rt_nb_sim_card_status_get(uint8_t *status)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    if (!status)
    {
        return -RT_ERROR;
    }

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_enter_pin(RIL_READ_MODE, 
                    NULL, NULL, _ri_request_common_cb, status);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset sim card status failed! ret:%d", ret);
    }
    else
    {
        LOG_D("[L:%d] waiting ril result...", __LINE__);
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
        if (local_ril_cmd_result != RT_NB_RIL_CMD_OK)
        {
            *status = 0;
        }

        LOG_D("sim card:%d", *status);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_cgpaddr_get(char *ip_addr, uint8_t len)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    int32_t cid[1] = {1};
    ril_show_pdp_address_req_t req;
    if (!ip_addr || len < RT_NB_MAX_IPADDR_LEN)
    {
        return -RT_ERROR;
    }

    req.cid_array_len = 1;
    req.cid_array = (int32_t*)cid;

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_show_pdp_address(RIL_EXECUTE_MODE, 
                    &req, _ri_request_common_cb, ip_addr);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset sim card status failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
        LOG_D("ip addr:%s", ip_addr);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return (ret == RIL_STATUS_SUCCESS && 
            local_ril_cmd_result == RT_NB_RIL_CMD_OK) ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_cclk_get(char *cclk, uint8_t len)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    rt_ext_bytes_t buf;

    if (!cclk || len < 1)
    {
        return -RT_ERROR;
    }

    buf.buf = cclk;
    buf.len = len;

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_clock(RIL_READ_MODE, 
                    NULL, _ri_request_common_cb, &buf);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset cclk failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
        LOG_D("cclk:%.*s", len, cclk);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return (ret == RIL_STATUS_SUCCESS && 
            local_ril_cmd_result == RT_NB_RIL_CMD_OK) ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_apn_get(char *apn_name, uint8_t len)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    char name[RT_NB_MAX_APN_NAME];
    if (!apn_name || len < 1)
    {
        return -RT_ERROR;
    }

    rt_memset(name, 0x0, sizeof(name));

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_define_pdp_context(RIL_READ_MODE, 
                    NULL, _ri_request_common_cb, name, -1);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset apn failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
        rt_strncpy(apn_name, name, len);
        LOG_D("apn name:%.*s", RT_NB_MAX_APN_NAME, name);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_apn_set(char *pdp_type, char *apn, char *username, char *password)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    ril_send_pdn_connection_set_default_psd_attach_rsp_t  req;
    if (!pdp_type)
    {
        return -RT_ERROR;
    }

    req.apn = apn;
    req.password = password;
    req.pdp_type = pdp_type;
    req.username = username;

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_pdn_connection_set_default_psd_attach(RIL_EXECUTE_MODE, 
                    &req, _ri_request_common_cb, NULL);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset apn failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_edrx_set(int32_t status, uint8_t *active_time)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_eDRX_setting(RIL_EXECUTE_MODE,
                    status, 5, (char *)active_time,
                    _ri_request_common_cb, NULL);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("set edrx failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_psm_set(int32_t status, uint8_t *tau, uint8_t *active_time)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    ril_power_saving_mode_setting_rsp_t rsp;

    rsp.mode = status;
    rsp.req_prdc_tau = (char *)tau;
    rsp.req_act_time = (char *)active_time;

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_power_saving_mode_setting(RIL_EXECUTE_MODE, 
                    &rsp, _ri_request_common_cb, NULL);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("set psm failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_cereg_get(rt_nb_network_registration_status_rsp_t  *cereg)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    ril_eps_network_registration_status_rsp_t rsp;
    if (!cereg)
    {
        return -RT_ERROR;
    }

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_eps_network_registration_status(RIL_READ_MODE, 
                    RIL_OMITTED_INTEGER_PARAM, _ri_request_common_cb, &rsp);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset sim card status failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);

        cereg->n = rsp.n;
        cereg->stat = rsp.stat;
        cereg->tac = rsp.tac;
        cereg->ci = rsp.ci;
        cereg->act = rsp.act;
        cereg->cause_type = rsp.cause_type;
        cereg->reject_cause = rsp.reject_cause;
        cereg->active_time = rsp.active_time;
        cereg->periodic_tau = rsp.periodic_tau;
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

rt_err_t rt_nb_cereg_set(uint8_t n)
{
    ril_status_t ret = RIL_STATUS_SUCCESS;
    ril_eps_network_registration_status_rsp_t rsp;

    /* Mutually exclusive operation for ril_request_xxx */
    rt_mutex_take(local_nb_ril_mutex, RT_WAITING_FOREVER);

    ret = ril_request_eps_network_registration_status(RIL_EXECUTE_MODE, 
                    n, _ri_request_common_cb, &rsp);
    if (ret != RIL_STATUS_SUCCESS)
    {
        LOG_E("requset sim card status failed! ret:%d", ret);
    }
    else
    {
        /* Blocking waiting for result */
        rt_sem_take(local_nb_ril_sync_sem, RT_WAITING_FOREVER);
    }

    rt_mutex_release(local_nb_ril_mutex);
    return ret == RIL_STATUS_SUCCESS ? RT_EOK : -RT_ERROR;
}

#ifdef RT_NB_DBG_FEATURE

static void _nb_cfg_sync(void)
{
    if (rt_nb_cfg_sync() != 0)
    {
        LOG_E("nb cfg sync failed!");
        return;
    }
    else
    {
        LOG_D("nb cfg sync pass");
    }
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(_nb_cfg_sync, nb_cfg_sync, nbiot cfg sync);
#endif /* FINSH_USING_MSH */

void rt_nb_cfg_dump(void)
{
    if (rt_nb_ril_init() != RT_EOK)
    {
        return;
    }

    /* query cpin status */
    {
        uint8_t sim_card_status;
        rt_nb_sim_card_status_get(&sim_card_status);
        rt_kprintf("sim card status:%d\n", sim_card_status);
    }

    if (rt_nb_cfg_sync() != 0)
    {
        LOG_E("nb cfg sync failed!");
        return;
    }
    rt_kprintf("sn    :%s\n", rt_nb_sn_get());
    rt_kprintf("imei  :%s\n", rt_nb_imei_get());
    rt_kprintf("imeisv:%s\n", local_nb_cfg.imeisv);
    rt_kprintf("svn   :%s\n", local_nb_cfg.svn);
    rt_kprintf("imsi  :%s\n", local_nb_cfg.imsi);
    rt_kprintf("iccid :%s\n", local_nb_cfg.iccid);

    int32_t rssi, ber;
    rt_nb_csq_get(&rssi, &ber);
    rt_kprintf("rssi  :%d\n", rssi);
    rt_kprintf("ber   :%d\n", ber);

    {
        rt_nb_signal_quality_rsp_t signal;
        rt_nb_cesq_get(&signal);

        rt_nb_network_registration_status_rsp_t cereg;

        rt_nb_cereg_set(5);
        rt_thread_mdelay(1000);

        rt_nb_cereg_get(&cereg);
        rt_kprintf( "n           :%d\n",   cereg.n);
        rt_kprintf( "stat        :%d\n",   cereg.stat);
        rt_kprintf( "tac         :%04x\n", cereg.tac);
        rt_kprintf( "ci          :%08x\n", cereg.ci);
        rt_kprintf( "act         :%d\n",   cereg.act);
        if (cereg.n >= 3)
        {
            rt_kprintf( "cause_type  :%d\n",   cereg.cause_type);
            rt_kprintf( "reject_cause:%d\n",   cereg.reject_cause);
        }
        rt_kprintf( "active_time :%08x\n", cereg.active_time);
        rt_kprintf( "periodic_tau:%08x\n", cereg.periodic_tau);
    }

    /* query apn name */
    {
        char apn_name[RT_NB_MAX_APN_NAME];
        rt_nb_apn_get(apn_name, RT_NB_MAX_APN_NAME);
        rt_kprintf("apn name: %s\n", apn_name);
    }

    /* query ip addr */
    {
        char ip_addr[RT_NB_MAX_IPADDR_LEN];
        rt_memset(ip_addr, 0x0, sizeof(ip_addr));
        rt_nb_cgpaddr_get(ip_addr, RT_NB_MAX_IPADDR_LEN);
        rt_kprintf("ip addr:%s\n", ip_addr);
    }

    /* query time */
    {
        char cclk_time[32];
        rt_nb_cclk_get(cclk_time, sizeof(cclk_time));
        rt_kprintf("cclk time:%s\n", cclk_time);
    }

    /* cfun */
    {
        int32_t status = 0;

        /* get */
        rt_kprintf("get...\n");
        rt_nb_cfun_set_get(&status, 0);
        rt_kprintf("get cfun: %d\n", status);

        /* set */
        rt_kprintf("set...\n");
        status = 0;
        rt_nb_cfun_set_get(&status, 1);
        rt_kprintf("set cfun: %d\n", status);

        rt_thread_mdelay(2000);

        /* get */
        rt_kprintf("get...\n");
        rt_nb_cfun_set_get(&status, 0);
        rt_kprintf("get cfun: %d\n", status);

        /* set */
        rt_kprintf("set...\n");
        status = 1;
        rt_nb_cfun_set_get(&status, 1);
        rt_kprintf("set cfun: %d\n", status);

        rt_thread_mdelay(2000);

        /* get */
        rt_kprintf("get...\n");
        rt_nb_cfun_set_get(&status, 0);
        rt_kprintf("get cfun: %d\n", status);
    }
}
MSH_CMD_EXPORT_ALIAS(rt_nb_cfg_dump, nb_cfg_dump, dump imei/imsi/iccid/sn ...);
#endif /* RT_NB_DBG_FEATURE */
