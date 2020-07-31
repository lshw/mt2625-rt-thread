/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-09     MurphyZhao        first version
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <rtthread.h>

#ifdef NB_TCP_DEMO
#ifdef RT_USING_SAL

#include <sys/socket.h> 
#include <netdb.h>
#include "sal_tls.h"
#include "sys/time.h"
#else

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"
#include "lwip/inet.h"

#endif /* RT_USING_SAL */

#define LOG_TAG              "demo.tcp"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

#define TCP_TEST_HOST    "www.rt-thread.com"
#define TCP_TEST_PORT    (80u)

#define TEST_BUFSZ       (1024u)

static const char *req_data = "GET /service/rt-thread.txt HTTP/1.1\r\n"
    "Host: www.rt-thread.com\r\n"
    "User-Agent: rtthread/4.0.1 rtt\r\n\r\n";

static char req_uri[128];
static int  req_port;

static void nb_tcp_demo(int argc, char** argv)
{
    int ret;

    int sock = -1;
    struct hostent *host;
    struct sockaddr_in server_addr;
    int bytes_received;
    char *recv_data;

    char ip_addr_buf[64];

    if ((argc != 1) && (argc != 3))
    {
        LOG_E("In param error");
        LOG_I("cmd: demo_tcp [<host> <port>]");
        LOG_I("eg:  demo_tcp");
        LOG_I("     demo_tcp 127.0.0.1 8080");
        return;
    }

    rt_memset(req_uri, 0x0, sizeof(req_uri));

    if (argc == 3)
    {
        rt_strncpy(req_uri, argv[1], rt_strlen(argv[1]));
        req_port = atoi(argv[2]);
    }
    else
    {
        rt_strncpy(req_uri, TCP_TEST_HOST, rt_strlen(TCP_TEST_HOST));
        req_port = TCP_TEST_PORT;
    }

    LOG_I("TCP demo start");

    LOG_I("Host:%s; Port:%d", req_uri, req_port);

    LOG_D("will gethostbyname...");
    host = gethostbyname(req_uri);
    if (!host)
    {
        LOG_E("gethostbyname failed!");
        return;
    }

    LOG_I("gethostbyname pass. ip addr: %s", inet_ntoa_r(*((struct in_addr *)host->h_addr_list[0]), ip_addr_buf, sizeof(ip_addr_buf)));

    recv_data = rt_calloc(1, TEST_BUFSZ);
    if (recv_data == RT_NULL)
    {
        LOG_E("calloc failed. No memory!");
        return;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_E("Create socket failed!");
        goto __exit;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(req_port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    if ((ret = connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) < 0)
    {
        LOG_E("Connect <%d> fail! ret:%d", sock, ret);
        goto __exit;
    }
    LOG_I("connect <%s> success", ip_addr_buf);

    ret = send(sock, req_data, strlen(req_data), 0);
    if (ret <= 0)
    {
        LOG_E("send error, will close the socket <%d>.", sock);
        goto __exit;
    }
    LOG_I("send success");

    bytes_received = recv(sock, recv_data, TEST_BUFSZ  - 1, 0);
    if (bytes_received <= 0)
    {
        LOG_E("receive error, will close the socket <%d>.", sock);
        goto __exit;
    }

    LOG_I("received data:\n");
    for (int i = 0; i < bytes_received; i++)
    {
        rt_kprintf("%c", recv_data[i]);
    }
    rt_kprintf("\r\n");

__exit:
    if (recv_data)
        rt_free(recv_data);

    if (sock >= 0)
    {
        closesocket(sock);
        sock = -1;
    }

    LOG_I("TCP demo end");
}
#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT_ALIAS(nb_tcp_demo, demo_tcp, nbiot tcp test);
#endif /* FINSH_USING_MSH */

#endif /* NB_TCP_DEMO */
