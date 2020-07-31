/*
 * This file is only applicable to mt2625 chip and is part of the onenet code of nb connection
 * Copyright (c) 2018-2030, Hbqs Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-11-01     longmain     first version
 * 2019-11-08     MurphyZhao   format
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "rtthread.h"
#include "rtdef.h"

#if defined(NB_USING_ONENET)

#include "nb_onenet.h"
#include "qs_cis_core.h"

#define LOG_TAG "nb.onenet"
#ifdef NB_ONENET_USING_LOG
#define LOG_LVL LOG_LVL_DBG
#else
#define LOG_LVL LOG_LVL_ERROR
#endif
#include <ulog.h>

//定义EVENT事件编号
#define event_bootstrap_fail        (1 << 1)
#define event_connect_fail          (1 << 2)
#define event_reg_success           (1 << 3)
#define event_reg_fail              (1 << 4)
#define event_reg_timeout           (1 << 5)
#define event_lifetime_timeout      (1 << 6)
#define event_update_success        (1 << 7)
#define event_update_fail           (1 << 8)
#define event_update_timeout        (1 << 9)
#define event_update_need           (1 << 10)
#define event_unreg_done            (1 << 11)
#define event_response_fail         (1 << 12)
#define event_response_success      (1 << 13)
#define event_notify_fail           (1 << 14)
#define event_notify_success        (1 << 15)
#define event_observer_success      (1 << 16)
#define event_discover_success      (1 << 17)

struct onenet_device
{
    int lifetime;
    int result;
    int dev_len;
    int initstep;
    int objcount;
    int inscount;
    int observercount;
    int discovercount;
    int event_status;
    int observer_status;
    int discover_status;
    int update_status;
    int update_time;
    int read_status;
    int write_status;
    int exec_status;
    int notify_status;
    int connect_status;
    int close_status;
    int notify_ack;
};

static rt_uint8_t config_hex[120];
struct onenet_device onenet_device_table = {0};
static struct onenet_stream onenet_stream_table[NB_ONENET_OBJECT_MAX_NUM] = {{0}};

static rt_event_t onenet_event = RT_NULL;

void onenet_event_callback(rt_int32_t id)
{
    switch (id)
    {
    case 1:
        LOG_D("Bootstrap start    ");
        onenet_device_table.event_status = nb_onenet_status_run;
        break;
    case 2:
        LOG_D("Bootstrap success  ");
        onenet_device_table.event_status = nb_onenet_status_run;
        break;
    case 3:
        LOG_E("Bootstrap failure,Device not registered on onenet");
        onenet_device_table.event_status = nb_onenet_status_failure;
        rt_event_send(onenet_event, event_bootstrap_fail);
        break;
    case 4:
        LOG_D("Connect success");
        onenet_device_table.event_status = nb_onenet_status_run;
        break;
    case 5:
        LOG_E("Connect failure");
        onenet_device_table.event_status = nb_onenet_status_failure;
        rt_event_send(onenet_event, event_connect_fail);
        break;
    case 6:
        LOG_D("Reg onenet success");
        onenet_device_table.event_status = nb_onenet_status_success;
        rt_event_send(onenet_event, event_reg_success);
        break;
    case 7:
        if (onenet_device_table.event_status != nb_onenet_status_failure)
        {
            LOG_E("Device not registered on onenet Or the onenet Device Auth_code parameter is set");
            onenet_device_table.event_status = nb_onenet_status_failure;
            rt_event_send(onenet_event, event_reg_fail);
        }
        break;
    case 8:
        LOG_D("Reg onenet timeout");
        onenet_device_table.event_status = nb_onenet_status_failure;
        rt_event_send(onenet_event, event_reg_timeout);
        break;
    case 9:
        LOG_D("Life_time timeout");
        break;
    case 10:
        LOG_D("Status halt");
        break;
    case 11:
        LOG_D("Update success");
        onenet_device_table.update_time = nb_onenet_status_update_success;
        rt_event_send(onenet_event, event_update_success);
        break;
    case 12:
        LOG_E("Update failure");
        onenet_device_table.update_time = nb_onenet_status_update_failure;
        rt_event_send(onenet_event, event_update_fail);
        break;
    case 13:
        LOG_E("Update timeout");
        onenet_device_table.update_time = nb_onenet_status_update_timeout;
        rt_event_send(onenet_event, event_update_timeout);
        break;
    case 14:
        LOG_D("Update need");
        onenet_device_table.update_time = nb_onenet_status_update_need;
        break;
    case 15:
        LOG_D("onenet Unreg success");
        onenet_device_table.connect_status = nb_onenet_status_failure;
        if (onenet_device_table.close_status == nb_onenet_status_close_start)
        {
            onenet_device_table.close_status = nb_onenet_status_close_init;
            rt_event_send(onenet_event, event_unreg_done);
        }
        else
        {
            if (nb_onenet_close_callback() != RT_EOK)
            {
                LOG_E("close onenet instance failure");
            }
        }
        break;
    case 20:
        LOG_E("Response failure");
        break;
    case 21:
        LOG_D("Response success");
        break;
    case 25:
        LOG_E("Notify failure");
        onenet_device_table.notify_status = nb_onenet_status_failure;
        rt_event_send(onenet_event, event_notify_fail);
        break;
    case 26:
        LOG_D("Notify success");
        onenet_device_table.notify_status = nb_onenet_status_success;
        rt_event_send(onenet_event, event_notify_success);
        break;
    case 40:
        LOG_D("enter onenet fota down start");
        break;
    case 41:
        LOG_E("onenet fota down fail");
        break;
    case 42:
        LOG_D("onenet fota down success");
        break;
    case 43:
        LOG_D("onenet enter fotaing ");

        break;
    case 44:
        LOG_D("onenet fota success");
        break;
    case 45:
        LOG_E("onenet fota failure");
        break;
    case 46:
        LOG_D("onenet fota update success");
        break;
    case 47:
        LOG_E("onenet fota event interrupt failure");
        break;
    default:
        break;
    }
}

int onenet_get_object_value_type(rt_uint32_t objid, rt_uint32_t insid, rt_uint32_t resid)
{
    rt_uint8_t i;
    for (i = 0; i < NB_ONENET_OBJECT_MAX_NUM; i++)
    {
        if (onenet_stream_table[i].objid == objid && onenet_stream_table[i].insid == insid && onenet_stream_table[i].resid == resid)
        {
            LOG_D("find  object type(%d)", onenet_stream_table[i].valuetype);
            return onenet_stream_table[i].valuetype;
        }
    }
    return -RT_ERROR;
}

int onenet_observer_rsp(rt_uint32_t objid, rt_uint32_t insid, rt_uint32_t resid, rt_uint32_t msgid)
{
    rt_uint8_t i;
    for (i = 0; i < NB_ONENET_OBJECT_MAX_NUM; i++)
    {
        if (onenet_stream_table[i].objid == objid && onenet_stream_table[i].insid == insid)
        {
            LOG_D("find observer object(%d)", objid);
            onenet_stream_table[i].msgid = msgid;
        }
    }
    LOG_D("find observer obj pass");
    // if (i > NB_ONENET_OBJECT_MAX_NUM)
    // {
    //     if (qs_cis_observer_rsp(msgid, 1) != RT_EOK)
    //     {
    //         LOG_E("onenet observer rsp failed");
    //         return -RT_ERROR;
    //     }
    // }
    if (qs_cis_observer_rsp(msgid, 0) != RT_EOK)
    {
        LOG_E("onenet observer rsp failed");
        return -RT_ERROR;
    }
    LOG_D("qs_cis_observer_rsp pass");

    onenet_device_table.observercount++;
    if (onenet_device_table.observercount == onenet_device_table.objcount)
    {
        //find observe success
        onenet_device_table.observercount = 0;
        onenet_device_table.observer_status = nb_onenet_status_success;
#ifdef NB_ONENET_USING_DEBUG
        LOG_D("observer success ");
#endif
        rt_event_send(onenet_event, event_observer_success);
    }

    LOG_D("onenet_observer_rsp pass");
    return RT_EOK;
}

int onenet_discover_rsp(rt_uint32_t msgid, rt_uint32_t objid)
{
    int *str_res;
    int i = 0, j = 0, status = 1, resourcecount = 0;
    str_res = rt_malloc(NB_ONENET_OBJECT_MAX_NUM);
    if (str_res == RT_NULL)
    {
        LOG_E("dicover rsp calloc fail");
        return -RT_ERROR;
    }
    for (i = 0; i < onenet_device_table.dev_len; i++)
    {
        if (objid == onenet_stream_table[i].objid)
        {
            for (j = 0; j < resourcecount; j++)
            {
                if (str_res[j] != onenet_stream_table[i].resid)
                    status = 1;
                else
                {
                    status = 0;
                    break;
                }
            }
            if (status)
            {
                str_res[resourcecount++] = i;
            }
        }
    }
    for (i = 0; i < resourcecount; i++)
    {
        if (i != resourcecount - 1)
        {
            status = 1;
        }
        else
        {
            status = 0;
        }
        if (qs_cis_discover_rsp(msgid, &onenet_stream_table[str_res[i]], status) != RT_EOK)
        {
            LOG_E("discover(%d  %d  %d) failed", onenet_stream_table[str_res[i]].objid, onenet_stream_table[str_res[i]].insid, onenet_stream_table[str_res[i]].resid);
            return -RT_ERROR;
        }
    }
    rt_free(str_res);
    onenet_device_table.discovercount++;
    onenet_device_table.discover_status = nb_onenet_status_run;

    if (onenet_device_table.discovercount == onenet_device_table.objcount)
    {
        //find discover success
        onenet_device_table.discovercount = 0;
        onenet_device_table.discover_status = nb_onenet_status_success;
        onenet_device_table.connect_status = nb_onenet_status_success;
        rt_event_send(onenet_event, event_discover_success);
    }
    return RT_EOK;
}

int onenet_read_rsp(int msgid, int objid, int insid, int resid)
{
    int i = 0;
    if (insid == -1 && resid == -1)
    {
        for (; i < onenet_device_table.dev_len; i++)
        {
            if (onenet_stream_table[i].objid == objid)
            {
                onenet_stream_table[i].read_status = 10;
            }
        }
    }
    else if (resid == -1)
    {
        for (; i < onenet_device_table.dev_len; i++)
        {
            if (onenet_stream_table[i].objid == objid && onenet_stream_table[i].insid == insid)
            {
                onenet_stream_table[i].read_status = 10;
            }
        }
    }
    else
    {
        for (; i < onenet_device_table.dev_len; i++)
        {
            if (onenet_stream_table[i].objid == objid && onenet_stream_table[i].insid == insid && onenet_stream_table[i].resid == resid)
            {
                onenet_stream_table[i].read_status = 10;
                break;
            }
        }
    }

    if (nb_onenet_read_rsp_callback(msgid, insid, resid) == RT_EOK)
    {
        LOG_D("onenet read rsp success");
        return RT_EOK;
    }
    LOG_E("onenet read rsp failure");
    return -RT_ERROR;
}

int onenet_write_rsp(int msgid, int objid, int insid, int resid, int valuetype, int len, nb_onenet_value_t value)
{
    int i = 0, status;
    LOG_D("write rsp:(%d/%d/%d) valuetype=%d mind=%d", objid, insid, resid, valuetype, msgid);
    for (; i < onenet_device_table.dev_len; i++)
    {
        if (onenet_stream_table[i].objid == objid && onenet_stream_table[i].insid == insid && onenet_stream_table[i].resid == resid)
        {
            onenet_stream_table[i].write_status = 10;
            status = nb_onenet_write_rsp_callback(len, value);
            onenet_stream_table[i].write_status = 0;
            if (status != RT_EOK)
            {
                LOG_E("onenet write rsp failure");
            }
            if (qs_cis_write_rsp(msgid) != RT_EOK)
            {
                LOG_E("onenet write rsp failed");
                return -RT_ERROR;
            }
            return RT_EOK;
        }
    }
    return -RT_ERROR;
}

int onenet_execute_rsp(int msgid, int objid, int insid, int resid, int len, char *cmd)
{
    int i = 0;
    for (; i < onenet_device_table.dev_len; i++)
    {
        if (onenet_stream_table[i].objid == objid && onenet_stream_table[i].insid == insid && onenet_stream_table[i].resid == resid)
        {
            onenet_stream_table[i].exec_status = 10;
            if (nb_onenet_exec_rsp_callback(len, cmd) == RT_EOK)
            {
                LOG_D("onenet execute rsp success");
            }
            else
            {
                LOG_E("onenet execute rsp failure");
            }
            onenet_stream_table[i].exec_status = 0;
            if (qs_cis_exec_rsp(msgid) != RT_EOK)
            {
                LOG_E("onenet exec rsp failed");
                return -RT_ERROR;
            }
            return RT_EOK;
        }
    }
    return -RT_ERROR;
}

int onenet_create_reg_code(rt_uint8_t *str)
{
    int ver = 1;
    int i = 0;
    int str_len = 0;
    int config1_start = 0;
    int config2_start = 0;
    int config2_stop = 0;
    unsigned int head = 0xf0;
    int mtu_size = 1280;
    int link_t = 1;
    int band_t = 1;
    // int boot_t = 0;
    int apn_len = 0;
    int user_name_len = 0;
    int passwd_len = 0;
    int host_len;
    str_len = rt_strlen((const char *)str);
    if (str_len)
    {
        rt_memset(str, 0, sizeof(str));
        str_len = 0;
    }
    for (i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            str[str_len++] = (ver << 4) + 3;
        }
        else
        {
            str[str_len++] = 0;
        }
    }
    config1_start = str_len - 1;
    str[str_len++] = (head + 1);
    str[str_len++] = 0x00;
    str[str_len++] = 0x03;
    for (i = 0; i < 3; i++)
    {
        if (i == 0)
        {
            str[str_len++] = head + 2;
        }
        else
        {
            str[str_len++] = 0;
        }
    }
    config2_start = str_len - 1;
    str[str_len++] = (mtu_size >> 8) & 0x0f;
    str[str_len++] = mtu_size & 0x0f;
    str[str_len++] = (link_t << 4) + band_t;

#ifndef nb_ONENET_BS_ON
    str[str_len++] = 00;
#else
    str[str_len++] = 128;
#endif
    if (apn_len == 0)
    {
        str[str_len++] = 00;
        str[str_len++] = 00;
    }
    if (user_name_len == 0)
    {
        str[str_len++] = 00;
        str[str_len++] = 00;
    }
    if (passwd_len == 0)
    {
        str[str_len++] = 00;
        str[str_len++] = 00;
    }
    host_len = rt_strlen(NB_ONENET_ADDRESS);
    str[str_len++] = 0;
    str[str_len++] = host_len;
    for (i = 0; i < host_len; i++)
    {
        str[str_len++] = NB_ONENET_ADDRESS[i];
    }
    str[str_len++] = 0;
    str[str_len++] = 0x04;
    str[str_len++] = 0x4e;
    str[str_len++] = 0x55;
    str[str_len++] = 0x4c;
    str[str_len++] = 0x4c;
    config2_stop = str_len;
    str[config2_start] = config2_stop - config2_start + 2;
    str[str_len++] = 0xf3;
    str[str_len++] = 0x00;
    str[str_len++] = 0x0d;
    str[str_len++] = 0xea;
    str[str_len++] = 0x04;
    str[str_len++] = 0x00;
    str[str_len++] = 0x00;
    str[str_len++] = 0x04;
    str[str_len++] = 0x4e;
    str[str_len++] = 0x55;
    str[str_len++] = 0x4c;
    str[str_len++] = 0x4c;
    str[config1_start] = str_len;

    return str_len;
}

/*
************************************************************
*	函数名称：	nb_onenet_init_environment
*
*	函数功能：	初始化ONENET运行环境
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
***********************************************************/
static int local_nb_onenet_inited = 0;
int nb_onenet_init_environment(void)
{
    if (local_nb_onenet_inited)
        return RT_EOK;

    onenet_event = rt_event_create("on_event", RT_IPC_FLAG_FIFO);
    if (!onenet_event)
    {
        LOG_E("onenet event create failed");
        return -RT_ERROR;
    }

    rt_memset(&onenet_device_table, 0, sizeof(onenet_device_table));

    onenet_device_table.dev_len = 0;
    onenet_device_table.lifetime = NB_ONENET_LIFE_TIME;
    onenet_device_table.connect_status = nb_onenet_status_init;
    onenet_device_table.initstep = 0;
    onenet_device_table.observercount = 0;
    onenet_device_table.discovercount = 0;

    rt_memset(onenet_stream_table, 0x0, sizeof(onenet_stream_table));

    local_nb_onenet_inited = 1;
    LOG_D("nb.onenet init success");

    return RT_EOK;
}

/*
************************************************************
*	函数名称：	nb_onenet_is_inited
*
*	函数功能：	判断 ONENET 环境是否已经初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		
***********************************************************/
rt_bool_t nb_onenet_is_inited(void)
{
    return local_nb_onenet_inited == 1;
}

/*************************************************************
*	函数名称：	nb_onenet_object_init
*
*	函数功能：	初始化一个onenet平台object数据流
*
*	入口参数：	objid:    object id
*						  insid:    instance id
*						  resid: 		resource id
*						  inscount: instance 数量
*						  bitmap:   instance bitmap
*						  atts: 		attribute count (具有Read/Write操作的object有attribute)
*						  actis:    action count (具有Execute操作的object有action)
*						  type: 		数据类型
*
*	返回参数：	object指针：成功		RT_NULL:失败
*
*	说明：		
************************************************************/
nb_onenet_stream_t nb_onenet_object_init(int objid, int insid, int resid, int inscount, char *bitmap, int atts, int acts, int type)
{
    int num = 0;
    for (num = 0; num < NB_ONENET_OBJECT_MAX_NUM && onenet_stream_table[num].user_status; num++)
        ;
    if (num >= NB_ONENET_OBJECT_MAX_NUM)
    {
        LOG_E("onenet stream max error");
        return RT_NULL;
    }
    onenet_stream_table[num].objid = objid;
    onenet_stream_table[num].insid = insid;
    onenet_stream_table[num].resid = resid;
    onenet_stream_table[num].inscount = inscount;
    onenet_stream_table[num].atts = atts;
    onenet_stream_table[num].acts = acts;
    onenet_stream_table[num].valuetype = type;
    onenet_stream_table[num].user_status = 1;
    onenet_device_table.dev_len++;
    rt_strncpy(onenet_stream_table[num].bitmap, bitmap, NB_ONENET_INSTANCE_MAX_NUM);
    return &onenet_stream_table[num];
}

/******************************************************
* 函数名称： onenet_create_object
*
*	函数功能： 添加 object 到模组
*
* 入口参数： 无
*
* 返回值： 0 成功  1失败
*
********************************************************/
static int onenet_create_object(void)
{
    int i = 0, j = 0, status = 1;
    int str[NB_ONENET_OBJECT_MAX_NUM];
    onenet_device_table.objcount = 0;

    for (; i < NB_ONENET_OBJECT_MAX_NUM; i++)
    {
        if (onenet_stream_table[i].user_status)
        {
            for (j = 0; j < onenet_device_table.objcount; j++)
            {
                if (str[j] != onenet_stream_table[i].objid)
                    status = 1;
                else
                {
                    status = 0;
                    break;
                }
            }
            if (status)
            {
                str[onenet_device_table.objcount++] = onenet_stream_table[i].objid;
                onenet_device_table.inscount = onenet_device_table.dev_len;
#ifdef NB_ONENET_USING_DEBUG
                LOG_D("onenet add object(%d)\"%s\",%d,%d", onenet_stream_table[i].objid, onenet_stream_table[i].inscount,
                      onenet_stream_table[i].bitmap, onenet_stream_table[i].atts, onenet_stream_table[i].acts);
#endif
                if (qs_cis_add_obj(&onenet_stream_table[i]) != RT_EOK)
                {
                    LOG_E("create object(%d) failed", onenet_stream_table[i].objid);
                }
                else
                {
                    LOG_D("create object success id:%d", onenet_stream_table[i].objid);
                }
            }
        }
    }
    return RT_EOK;
}

/******************************************************
* 函数名称： nb_onenet_delete_object
*
*	函数功能： 删除模组内已注册 object 数据流
*
* 入口参数： stream : object编号
*
* 返回值： 0：成功   1：失败
*
********************************************************/
int nb_onenet_delete_object(nb_onenet_stream_t stream)
{
    int i = 0, result = 0;
    for (i = 0; i < NB_ONENET_OBJECT_MAX_NUM; i++)
    {
        if (onenet_stream_table[i].objid == stream->objid)
        {
#ifdef NB_ONENET_USING_DEBUG
            LOG_D("onenet del objid(%d)", onenet_stream_table[i].objid);
#endif
            rt_memset(&onenet_stream_table[i], 0, sizeof(onenet_stream_table[i]));
            onenet_device_table.dev_len -= 1;
            onenet_device_table.objcount -= 1;
            result = 1;
        }
    }
    if (result == 1)
    {
        i = (i == NB_ONENET_OBJECT_MAX_NUM) ? i - 1 : i;
        if (qs_cis_del_obj(&onenet_stream_table[i]) != RT_EOK)
        {
            LOG_E("del object(%d) failed", onenet_stream_table[i].objid);
        }
        LOG_D("delete onenet object success id:%d ", stream->objid);
        return RT_EOK;
    }
    return -RT_ERROR;
}

/******************************************************
* 函数名称： onenet_create_instance
*
*	函数功能： 创建模组实例 instance
*
* 入口参数： config_t 设备注册码
*
* 返回值： 0 成功  1失败
*
********************************************************/
static int onenet_create_instance(void)
{
    rt_uint32_t len = 0;
    len = onenet_create_reg_code(config_hex);

    if (qs_cis_init(config_hex, len) != RT_EOK)
    {
        LOG_E("create init failed");
        return -RT_ERROR;
    }
    return RT_EOK;
}

/******************************************************
* 函数名称： nb_onenet_delete_instance
*
*函数功能： 删除模组实例 instance
*
* 入口参数： 无
*
* 返回值：	 0：成功   1：失败
*
********************************************************/
int nb_onenet_delete_instance(void)
{
#ifdef NB_ONENET_USING_DEBUG
    LOG_D("onenet del instance");
#endif
    if (qs_cis_deinit() != RT_EOK)
    {
        LOG_E("deinit onenet failed");
        return -RT_ERROR;
    }
    rt_memset(&onenet_device_table, 0, sizeof(onenet_device_table));
    rt_memset(&onenet_stream_table, 0, sizeof(onenet_stream_table));

    LOG_D("onenet instace delete success");
    return RT_EOK;
}

/****************************************************
* 函数名称： nb_onenet_open
*
* 函数作用： 设备登录到 onenet 平台
*
* 入口参数： 无
*
* 返回值： 0：成功   1：失败
*****************************************************/
int nb_onenet_open(void)
{
    rt_uint32_t status;

#ifdef NB_ONENET_USING_DEBUG
    LOG_D("onenet open lifetime=,%d", onenet_device_table.lifetime);
#endif
    if (qs_cis_reg(onenet_device_table.lifetime) != RT_EOK)
    {
        LOG_E("onenet open failed");
        return -RT_ERROR;
    }
    onenet_device_table.initstep = 4;
    onenet_device_table.event_status = nb_onenet_status_init;
    if (rt_event_recv(onenet_event, event_bootstrap_fail | event_connect_fail | event_reg_fail | event_reg_success | event_reg_timeout, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 3000, &status) != RT_EOK)
    {
        LOG_E("onenet wait reg timeout");
        return -RT_ERROR;
    }
    if (status == event_bootstrap_fail || status == event_connect_fail || status == event_reg_fail)
    {
        LOG_E("onenet reg failure");
        return -RT_ERROR;
    }
    if (status == event_reg_timeout)
    {
        LOG_E("The device is not registered in the platform or filled in Auto Code");
        return -RT_ERROR;
    }
    onenet_device_table.initstep = 5;
    if (rt_event_recv(onenet_event, event_observer_success, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 6000, &status) != RT_EOK)
    {
        LOG_E("wait onenet observe event timeout");
        return -RT_ERROR;
    }
    onenet_device_table.initstep = 6;
    if (rt_event_recv(onenet_event, event_discover_success, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 6000, &status) != RT_EOK)
    {
        LOG_E("Please turn on auto discovery resource in onenet platform device management");
        return -RT_ERROR;
    }
    if (onenet_device_table.discover_status == nb_onenet_status_success)
    {
        LOG_D("onenet lwm2m connect success");
        onenet_device_table.initstep = 13;
        return RT_EOK;
    }
    else
    {
        LOG_E("discover failure,Please turn on auto discovery resource in onenet platform device management");
        return -RT_ERROR;
    }
}

/****************************************************
* 函数名称： nb_onenet_close
*
* 函数作用： 在onenet 平台注销设备
*
* 入口参数： 无
*
* 返回值： 0：成功   1：失败
*****************************************************/
int nb_onenet_close(void)
{
    rt_uint32_t status;
    onenet_device_table.close_status = nb_onenet_status_close_start;
#ifdef NB_ONENET_USING_DEBUG
    LOG_D("onenet will close");
#endif
    if (qs_cis_unreg() != RT_EOK)
    {
        LOG_E("onenet close failed");
        return -RT_ERROR;
    }
    if (rt_event_recv(onenet_event, event_unreg_done, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 20000, &status) != RT_EOK)
    {
        LOG_E("close onenet instance failure");
        return -RT_ERROR;
    }
    LOG_D("close onenet instance success");
    onenet_device_table.connect_status = nb_onenet_status_init;
    return RT_EOK;
}

/****************************************************
* 函数名称： nb_onenet_update_time
*
* 函数作用： 更新onenet 设备维持时间
*
* 入口参数： flge :是否同时更新云端object 信息
*
* 返回值： 0：成功   1：失败
*****************************************************/
int nb_onenet_update_time(int flge)
{
    rt_uint32_t status;
    onenet_device_table.update_time = nb_onenet_status_update_init;
#ifdef NB_ONENET_USING_DEBUG
    LOG_D("onenet update time=%d,%d", onenet_device_table.lifetime, flge);
#endif
    if (qs_cis_update_lifetime(onenet_device_table.lifetime, flge) != RT_EOK)
    {
        LOG_E("onenet update lifetime failed");
        goto __exit;
    }
    if (rt_event_recv(onenet_event, event_lifetime_timeout | event_update_fail | event_update_timeout | event_update_success, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 20000, &status) != RT_EOK)
    {
        goto __exit;
    }
    if (status == event_lifetime_timeout || status == event_update_fail || status == event_update_timeout)
    {
        goto __exit;
    }
    LOG_D("onenet update time success");
    return RT_EOK;

__exit:
    LOG_E("onenet update time failure");
    return -RT_ERROR;
}

/*************************************************************
*	函数名称：	nb_onenet_quick_start
*
*	函数功能：	一键连接到onenet平台
*
*	入口参数：	无
*
*	返回参数：	RT_EOK：成功   -RT_ERROR：失败
*
*	说明：		
************************************************************/
int nb_onenet_quick_start(void)
{
    if (onenet_create_instance() != RT_EOK)
    {
        LOG_E("onenet create instance failure");
        goto failure;
    }
    onenet_device_table.initstep = 1;
    rt_thread_mdelay(1000);
    if (onenet_create_object() != RT_EOK)
    {
        LOG_E("onenet add objece failure");
        return -RT_ERROR;
    }
    onenet_device_table.initstep = 2;

    if (nb_onenet_open() != RT_EOK)
        goto failure;

    return RT_EOK;

failure:
    if (onenet_device_table.initstep > 5)
    {
        nb_onenet_close();
    }
    return -RT_ERROR;
}

/****************************************************
* 函数名称： nb_onenet_notify
*
* 函数作用： notify 设备数据到平台（无ACK）
*
* 入口参数： stream：object结构体		len：消息长度		data：消息内容		flge：消息标识		
*
* 返回值： 0 成功	1失败
*****************************************************/
int nb_onenet_notify(nb_onenet_stream_t stream, int len, nb_onenet_value_t data, int flge)
{
#ifdef NB_ONENET_USING_LOG
    LOG_D("notify data(%d %d %d),", stream->objid, stream->insid, stream->resid);
    switch (stream->valuetype)
    {
    case nb_onenet_value_string:
    {
        LOG_D("%s %d", data->string_value, flge);
    }
    break;
    case nb_onenet_value_opaque:
    {
        LOG_D("%s %d", data->string_value, flge);
    }
    break;
    case nb_onenet_value_integer:
    {
        LOG_D("%d %d", data->int_value, flge);
    }
    break;
    case nb_onenet_value_float:
    {
        LOG_D("float: %.4f %d", (double)data->float_value, flge);
    }
    break;
    case nb_onenet_value_bool:
    {
        LOG_D("%d %d", data->bool_value, flge);
    }
    break;
    default:
        break;
    }
#endif

    if (qs_cis_notify(stream, len, data, flge) != RT_EOK)
    {
        LOG_E("onenet notify failed");
        return -RT_ERROR;
    }
    return RT_EOK;
}

/****************************************************
* 函数名称： qsdk_onenet_notify_and_ack
*
* 函数作用： notify 数据到onenet平台（带ACK）
*
* 入口参数： stream：object结构体		len：消息长度		data：消息内容		flge：消息标识	
*
* 返回值： 0 成功	1失败
*****************************************************/
int nb_onenet_notify_with_ack(nb_onenet_stream_t stream, int len, nb_onenet_value_t data, int flge)
{
    rt_uint32_t status = 0;
#ifdef NB_ONENET_USING_LOG
    LOG_D("notify data(%d %d %d),", stream->objid, stream->insid, stream->resid);
    switch (stream->valuetype)
    {
    case nb_onenet_value_string:
    {
        LOG_D("%s %d", data->string_value, flge);
    }
    break;
    case nb_onenet_value_opaque:
    {
        LOG_D("%s %d", data->string_value, flge);
    }
    break;
    case nb_onenet_value_integer:
    {
        LOG_D("%d %d", data->int_value, flge);
    }
    break;
    case nb_onenet_value_float:
    {
        LOG_D("%.4f %d", (double)data->float_value, flge);
    }
    break;
    case nb_onenet_value_bool:
    {
        LOG_D("%d %d", data->bool_value, flge);
    }
    break;
    default:
        break;
    }
#endif

    if (qs_cis_notify_with_ack(stream, len, data, flge, onenet_device_table.notify_ack) != RT_EOK)
    {
        LOG_E("onenet notify failed");
        return -RT_ERROR;
    }
    rt_event_recv(onenet_event, event_notify_fail | event_notify_success, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 20000, &status);
    if (status == event_notify_fail)
    {
        LOG_E("ack notify to onenet failure, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d", stream->objid,
              stream->insid, stream->resid, stream->msgid);
        onenet_device_table.notify_status = nb_onenet_status_init;
        return -RT_ERROR;
    }

#ifdef NB_ONENET_USING_LOG
    LOG_D("ack notify to onenet success, objectid:%d 	instanceid:%d 	resourceid:%d		msgid:%d", stream->objid, stream->insid, stream->resid, stream->msgid);
#endif
    onenet_device_table.notify_status = nb_onenet_status_init;
    onenet_device_table.notify_ack++;
    if (onenet_device_table.notify_ack > 5000)
        onenet_device_table.notify_ack = 1;
    return RT_EOK;
}

/****************************************************
* 函数名称： nb_onenet_get_connect
*
* 函数作用： 获取onenet 平台连接状态
*
* 入口参数： 无
*
* 返回值： 0：连接成功	     1：连接已断开
*****************************************************/
int nb_onenet_get_connect(void)
{
    if (onenet_device_table.connect_status == nb_onenet_status_success)
        return RT_EOK;

    return -RT_ERROR;
}

/****************************************************
* 函数名称： nb_onenet_get_object_read
*
* 函数作用： 查询当前object 是否收到平台read消息
*
* 入口参数： stream：object结构体
*
* 返回值： 0：收到	  1：没收到
*****************************************************/
int nb_onenet_get_object_read(nb_onenet_stream_t stream)
{
    if (stream->read_status == 10)
        return RT_EOK;

    return -RT_ERROR;
}

/****************************************************
* 函数名称： nb_onenet_get_object_write
*
* 函数作用： 查询当前object 是否收到平台write消息
*
* 入口参数： stream：object结构体
*
* 返回值： 0：收到	  1：没收到
*****************************************************/
int nb_onenet_get_object_write(nb_onenet_stream_t stream)
{
    if (stream->write_status == 10)
        return RT_EOK;

    return -RT_ERROR;
}

/****************************************************
* 函数名称： nb_onenet_get_object_exec
*
* 函数作用： 查询当前object 是否收到平台exec消息
*
* 入口参数： object：object分组	
*
* 返回值： 0：收到	  1：没收到
*****************************************************/
int nb_onenet_get_object_exec(nb_onenet_stream_t stream)
{
    if (stream->exec_status == 10)
        return RT_EOK;

    return -RT_ERROR;
}

/****************************************************
* 函数名称： nb_onenet_read_rsp
*
* 函数作用： 响应onenet read 操作
*
* 入口参数： msgid：消息ID		
*
*			result：读写状态
*	
*			stream：object 结构体
*
*			len：上报的消息长度
*
*			data：上报的消息值			
*
*			flge: 消息标识
*
* 返回值： 0：成功   1：失败
*****************************************************/
int nb_onenet_read_rsp(int msgid, int result, nb_onenet_stream_t stream, int len, nb_onenet_value_t data, int flge)
{
    if (qs_cis_read_rsp(msgid, result, stream, data, flge) != RT_EOK)
    {
        LOG_E("onenet read rsp failed");
        return -RT_ERROR;
    }
    stream->read_status = 0;
    return RT_EOK;
}

int nb_rsp_onenet_parameter(int instance, int msgid, int result)
{

    return 0;
}

INIT_ENV_EXPORT(nb_onenet_init_environment);

#endif /* NB_USING_ONENET */
