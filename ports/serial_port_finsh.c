
#ifdef MTK_PORT_SERVICE_ENABLE
#include "serial_port.h"
#include "serial_port_internal.h"

#include "rtthread.h"

static serial_port_register_callback_t g_serial_port_finsh_callback[1] = {NULL};

#ifdef NB_USING_SOFTWARE_AT_CMD
static serial_port_dev_t connl_dev = SERIAL_PORT_DEV_UNDEFINED;
static volatile uint8_t at_software_operate = 0;

#ifndef NB_SOFTWARE_AT_CMD_BUF_SIZE
#define NB_SOFTWARE_AT_CMD_BUF_SIZE (128u)
#endif
static char at_software_cmd[NB_SOFTWARE_AT_CMD_BUF_SIZE];

void at_cmd_send_by_software(int argc, char** argv) //(const char *cmd)
{
    char *cmd;

    if (argc != 2)
    {
        rt_kprintf("[soft at] In param error\n");
        return;
    }
    cmd = (char*)argv[1];

    if (at_software_operate)
    {
        return;
    }
    at_software_operate = 1;

    snprintf(at_software_cmd, sizeof(at_software_cmd), "%s\r\n", cmd);

    // call at serial callback with recv event
    if (g_serial_port_finsh_callback[0])
    {
        g_serial_port_finsh_callback[0](
            SERIAL_PORT_DEV_FINSH, SERIAL_PORT_EVENT_READY_TO_READ, NULL);
    }
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(at_cmd_send_by_software, at_cmd_exec, exec at cmd);
#endif
#endif /* NB_USING_SOFTWARE_AT_CMD */

/* Open rt-thread finsh port */
serial_port_status_t serial_port_finsh_init(serial_port_dev_t device, serial_port_open_para_t *para, void *priv_data)
{
    g_serial_port_finsh_callback[0] = para->callback;

    return SERIAL_PORT_STATUS_OK;
}

serial_port_status_t serial_port_finsh_control(serial_port_dev_t dev, serial_port_ctrl_cmd_t cmd, serial_port_ctrl_para_t *para)
{
    serial_port_status_t ret = SERIAL_PORT_STATUS_OK;
    serial_port_write_data_t *serial_port_write_data;
    serial_port_read_data_t *serial_port_read_data;
    serial_port_get_write_avail_t *serial_port_get_write_avail;
    serial_port_get_read_avail_t *serial_port_get_read_avail;

    if (!para)
    {
        rt_kprintf("[finsh] in param error\n");
        return SERIAL_PORT_STATUS_UNSUPPORTED;
    }

    switch (cmd) {
        case SERIAL_PORT_CMD_WRITE_DATA:
            serial_port_write_data = (serial_port_write_data_t *)para;

#ifdef NB_USING_SOFTWARE_AT_CMD
            rt_kprintf("%.*s", 
                serial_port_write_data->size, 
                serial_port_write_data->data);
#endif
            serial_port_write_data->ret_size = serial_port_write_data->size;
            break;
        case SERIAL_PORT_CMD_READ_DATA:
            serial_port_read_data = (serial_port_read_data_t *)para;
#ifdef NB_USING_SOFTWARE_AT_CMD
            if (at_software_operate)
            {
                strncpy((char *)serial_port_read_data->buffer, at_software_cmd, strlen(at_software_cmd));
                serial_port_read_data->ret_size = strlen(at_software_cmd);
                at_software_operate = 0;
            }
            else
            {
                serial_port_read_data->ret_size = serial_port_read_data->size;
            }
#else
            serial_port_read_data->ret_size = serial_port_read_data->size;
#endif
            break;
        case SERIAL_PORT_CMD_WRITE_DATA_BLOCKING:
            serial_port_write_data = (serial_port_write_data_t *)para;
            serial_port_write_data->ret_size = serial_port_write_data->size;
            break;
        case SERIAL_PORT_CMD_READ_DATA_BLOCKING:
            serial_port_read_data = (serial_port_read_data_t *)para;
            serial_port_read_data->ret_size = serial_port_read_data->size;
            break;
        case SERIAL_PORT_CMD_GET_WRITE_AVAIL:
            serial_port_get_write_avail = (serial_port_get_write_avail_t *)para;
            serial_port_get_write_avail->ret_size = 0;
            break;
        case SERIAL_PORT_CMD_GET_READ_AVAIL:
            serial_port_get_read_avail = (serial_port_get_read_avail_t *)para;
            serial_port_get_read_avail->ret_size = 0;
            break;
        default:
            ret = SERIAL_PORT_STATUS_UNSUPPORTED;
    }
    return ret;
}

/* Close rt-thread finsh port */
serial_port_status_t serial_port_finsh_deinit(serial_port_dev_t port)
{
    return SERIAL_PORT_STATUS_OK;
}
#endif
