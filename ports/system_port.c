/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-24     MurphyZhao   First version
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "serial_port.h"

#include <rtthread.h>

#define LOG_TAG              "main"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

static volatile uint8_t mtk_sys_init_done = 0;

void mtk_system_ready(void)
{
    mtk_sys_init_done = 1;
}

/**
 * 0: not ready
 * 1: ready
*/
uint8_t mtk_system_is_ready(void)
{
    return mtk_sys_init_done;
}


// dump uart config
void rt_nb_serial_dump(void)
{
    uint32_t serial_dev_num = 0;
    serial_port_assign_t serial_port_assign[SERIAL_PORT_DEV_MAX];
    if (serial_port_config_dump_dev_number(&serial_dev_num, serial_port_assign) != SERIAL_PORT_STATUS_OK)
    {
        LOG_E("serial dump failed");
    }

    rt_kprintf("=== serial device %d:%d ===\n", serial_dev_num, SERIAL_PORT_DEV_MAX);
    for (int i = 0; i < serial_dev_num; i ++)
    {
        rt_kprintf("[%d] %8s %d\n", i, serial_port_assign[i].name, (int)serial_port_assign[i].device);
    }
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(rt_nb_serial_dump, nb_serial_num_dump, dump serial device num);
#endif

// read uart config
void rt_nb_serial_dev_num_read(int argc, char** argv)
{
    serial_port_dev_t device;
    if (argc != 2)
    {
        LOG_E("in param error");
        return;
    }

    if (serial_port_config_read_dev_number(argv[1], &device) != SERIAL_PORT_STATUS_OK)
    {
        LOG_E("serial dev num read failed");
        return;
    }

    rt_kprintf("%s:%d\n", argv[1], device);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(rt_nb_serial_dev_num_read, nb_serial_num_read, nb_serial_num_read <dev name>: read serial num by name);
#endif

/**
 * reconfig uart device number 
 * 该接口目前没有起作用，因为 sys_init.c 中的 system_init 函数
 * 会强制将 nvdm 中的配置重定义为程序中默认的配置。这么做的目的是禁止用户修改串口绑定关系，
 * 以阻止潜在的串口配置冲突导致的宕机。
 * 
*/
void rt_nb_serial_dev_num_write(int argc, char** argv)
{
    serial_port_dev_t device;
    if (argc != 3)
    {
        LOG_E("in param error");
        return;
    }

    device = (serial_port_dev_t)atoi(argv[2]);

    if (serial_port_config_write_dev_number(argv[1], device) != SERIAL_PORT_STATUS_OK)
    {
        LOG_E("serial dev num write failed");
        return;
    }
    LOG_I("serial dev num write pass");
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(rt_nb_serial_dev_num_write, nb_serial_num_write, nb_serial_num_write <dev name> <dev num>);
#endif
