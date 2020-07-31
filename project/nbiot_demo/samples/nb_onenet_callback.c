/*
 * This file is part of onenet in nb
 * Copyright (c) 2018-2030, Hbqs Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-11-01     longmain     first version
 * 2019-11-08     MurphyZhao   format
 */

#include <rtthread.h>

#if defined(NB_USING_ONENET) && defined(NB_ONENET_DEMO)

#include "nb_onenet.h"

#define LOG_TAG "onenet.callback"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

/**
 * defined in demo_onenet.c
*/
extern nb_onenet_stream_t temp_object;
extern nb_onenet_stream_t hump_object;
extern nb_onenet_stream_t light0_object;
extern nb_onenet_stream_t inter_object;
extern nb_onenet_stream_t opaque_object;
extern nb_onenet_stream_t float_object;
extern nb_onenet_stream_t string_object;

static rt_bool_t light_value = RT_TRUE;
static rt_uint32_t int_value = 99;
static const char *string_value = "123456";
static const char *opaque_value = "abcde";
static float float_value = 99.6;

/****************************************************
* 函数名称： nb_onenet_close_callback
*
* 函数作用： onenet平台强制断开连接回调函数
*
* 入口参数： 无
*
* 返回值： 0 处理成功	1 处理失败
*****************************************************/
int nb_onenet_close_callback()
{
    LOG_I("enter close onenent callback");

    return RT_EOK;
}

/****************************************************
* 函数名称： nb_onenet_read_rsp_callback
*
* 函数作用： onenet平台 read操作回调函数
*
* 入口参数： msgid：消息ID	insid：instance id	resid: resource id
*
* 返回值： 0 处理成功	1 处理失败
*****************************************************/
int nb_onenet_read_rsp_callback(int msgid, int insid, int resid)
{
    float temp_value = 45.6, hump_value = 12.5;
    LOG_I("enter read rsp callback");
    if (insid == -1 && resid == -1)
    {
        LOG_D("will read all instance");
        if (nb_onenet_get_object_read(temp_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, temp_object, 0, (nb_onenet_value_t)&temp_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(hump_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, hump_object, 0, (nb_onenet_value_t)&hump_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(light0_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, light0_object, 0, (nb_onenet_value_t)&light_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(inter_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, inter_object, 0, (nb_onenet_value_t)&int_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(opaque_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, opaque_object, rt_strlen((const char *)opaque_value), (nb_onenet_value_t)&opaque_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(float_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, float_object, 0, (nb_onenet_value_t)&float_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, string_object, rt_strlen((const char *)string_value), (nb_onenet_value_t)&string_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
    }
    else if (resid == -1)
    {
        LOG_D("will read all resource");
        if (nb_onenet_get_object_read(temp_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, temp_object, 0, (nb_onenet_value_t)&temp_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(hump_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, hump_object, 0, (nb_onenet_value_t)&hump_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(light0_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, light0_object, 0, (nb_onenet_value_t)&light_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(inter_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, inter_object, 0, (nb_onenet_value_t)&int_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(opaque_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, opaque_object, rt_strlen((const char *)opaque_value), (nb_onenet_value_t)&opaque_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(float_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, float_object, 0, (nb_onenet_value_t)&float_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, string_object, rt_strlen((const char *)string_value), (nb_onenet_value_t)&string_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
    }
    else
    {
        LOG_D("will read one instance and one resource");
        if (nb_onenet_get_object_read(temp_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, temp_object, 0, (nb_onenet_value_t)&temp_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(hump_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, hump_object, 0, (nb_onenet_value_t)&hump_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(light0_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, light0_object, 0, (nb_onenet_value_t)&light_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(inter_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, inter_object, 0, (nb_onenet_value_t)&int_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(opaque_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, opaque_object, rt_strlen((const char *)opaque_value), (nb_onenet_value_t)&opaque_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(float_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, float_object, 0, (nb_onenet_value_t)&float_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
        else if (nb_onenet_get_object_read(string_object) == RT_EOK)
        {
            if (nb_onenet_read_rsp(msgid, nb_onenet_status_result_read_success, string_object, rt_strlen((const char *)string_value), (nb_onenet_value_t)&string_value, 0) != RT_EOK)
            {
                LOG_D("read rsp callback  failed");
            }
        }
    }

    return RT_EOK;
}

/****************************************************
* 函数名称： nb_onenet_write_rsp_callback
*
* 函数作用： onenet平台 write操作回调函数
*
* 入口参数： len:	需要写入的数据长度
*
*						 value:	需要写入的数据内容
*
* 返回值： 0 处理成功	1 处理失败
*****************************************************/
int nb_onenet_write_rsp_callback(int len, nb_onenet_value_t value)
{
    LOG_I("enter write dsp callback");
    if (nb_onenet_get_object_write(light0_object) == RT_EOK)
    {
        LOG_D("write light0 ,value=%d", value->bool_value);
    }
    else if (nb_onenet_get_object_write(temp_object) == RT_EOK)
    {
        LOG_D("write temp ,value=%.4f", (double)value->float_value);
    }
    else if (nb_onenet_get_object_write(hump_object) == RT_EOK)
    {
        LOG_D("write hump ,value=%.4f", (double)value->float_value);
    }
    else if (nb_onenet_get_object_write(inter_object) == RT_EOK)
    {
        LOG_D("write int ,value=%d", value->int_value);
    }
    else if (nb_onenet_get_object_write(opaque_object) == RT_EOK)
    {
        LOG_D("write opaque ,value=%s", value->string_value);
    }
    else if (nb_onenet_get_object_write(float_object) == RT_EOK)
    {
        LOG_D("write float ,value=%.4f", (double)value->float_value);
    }
    else if (nb_onenet_get_object_write(string_object) == RT_EOK)
    {
        LOG_D("write string ,value=%s", value->string_value);
    }
    return RT_EOK;
}

/****************************************************
* 函数名称： nb_onenet_exec_rsp_callback
*
* 函数作用： onenet平台 exec操作回调函数
*
* 入口参数： len:	平台exec命令下发数据长度
*
*						 cmd:	平台exec命令下发数据内容
*
* 返回值： 0 处理成功	1 处理失败
*****************************************************/
int nb_onenet_exec_rsp_callback(int len, char *cmd)
{
    LOG_I("enter exec dsp callback");
    LOG_D("len=%d,cmd=%s", len, cmd);
    return RT_EOK;
}

/****************************************************
* 函数名称： nb_onenet_fota_callback
*
* 函数作用： onenet 平台FOTA升级回调函数
*
* 入口参数： 无
*
* 返回值： 	 无
*****************************************************/
void nb_onenet_fota_callback(void)
{
    LOG_I("enter fota callback");
}
#endif /* NB_USING_ONENET && NB_ONENET_DEMO */
