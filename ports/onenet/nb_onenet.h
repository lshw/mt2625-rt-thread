/*
 * This file is part of the nbiot connection to the onenet platform
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-01     longmain     first version
 * 2019-11-08     MurphyZhao   format
 */

#ifndef __NB_ONENET_H__
#define __NB_ONENET_H__

#include <rtthread.h>

//nb_onenet_fun
enum nb_onenet_value_type
{
    nb_onenet_value_string = 1,
    nb_onenet_value_opaque,
    nb_onenet_value_integer,
    nb_onenet_value_float,
    nb_onenet_value_bool,
};

enum nb_onenet_status_type
{
    nb_onenet_status_init = 0,
    nb_onenet_status_run,
    nb_onenet_status_failure,
    nb_onenet_status_success = 4,
    nb_onenet_status_close_init = 0,
    nb_onenet_status_close_start,
    nb_onenet_status_update_init = 10,
    nb_onenet_status_update_failure,
    nb_onenet_status_update_success,
    nb_onenet_status_update_timeout = 14,
    nb_onenet_status_update_need = 18,
    nb_onenet_status_result_read_success = 1,
    nb_onenet_status_result_write_success,
    nb_onenet_status_result_Bad_Request = 11,
    nb_onenet_status_result_Unauthorized,
    nb_onenet_status_result_Not_Found,
    nb_onenet_status_result_Method_Not_Allowed,
    nb_onenet_status_result_Not_Acceptable
};

union nb_onenet_value {
    char *string_value;
    rt_uint16_t int_value;
    float float_value;
    rt_bool_t bool_value;
};

typedef union nb_onenet_value *nb_onenet_value_t;

struct onenet_stream
{
    int objid;
    int inscount;
    char bitmap[NB_ONENET_INSTANCE_MAX_NUM];
    int atts;
    int acts;
    int insid;
    int resid;
    int valuetype;
    int msgid;
    int read_status;
    int write_status;
    int exec_status;
    int user_status;
};

typedef struct onenet_stream *nb_onenet_stream_t;

int nb_onenet_init_environment(void);
rt_bool_t nb_onenet_is_inited(void);
nb_onenet_stream_t nb_onenet_object_init(int objid, int insid, int resid, int inscount, char *bitmap, int atts, int acts, int type);
int nb_onenet_delete_instance(void);
int nb_onenet_delete_object(nb_onenet_stream_t stream);
int nb_onenet_open(void);
int nb_onenet_close(void);
int nb_onenet_update_time(int flge);
int nb_onenet_quick_start(void);
int nb_onenet_notify(nb_onenet_stream_t stream, int len, nb_onenet_value_t data, int flge);
int nb_onenet_notify_with_ack(nb_onenet_stream_t stream, int len, nb_onenet_value_t data, int flge);
int nb_onenet_get_connect(void);
int nb_onenet_get_object_read(nb_onenet_stream_t stream);
int nb_onenet_get_object_write(nb_onenet_stream_t stream);
int nb_onenet_get_object_exec(nb_onenet_stream_t stream);
int nb_onenet_read_rsp(int msgid, int result, nb_onenet_stream_t stream, int len, nb_onenet_value_t data, int flge);

//callback fun
int nb_onenet_close_callback(void);
int nb_onenet_read_rsp_callback(int msgid, int insid, int resid);
int nb_onenet_write_rsp_callback(int len, nb_onenet_value_t value);
int nb_onenet_exec_rsp_callback(int len, char *cmd);
void nb_onenet_fota_callback(void);

#endif /* __NB_ONENET_H__ */
