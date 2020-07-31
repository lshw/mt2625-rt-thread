#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 24
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_256
#define RT_THREAD_PRIORITY_MAX 256
#define RT_TICK_PER_SECOND 100
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDEL_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 2048
#define RT_USING_TIMER_SOFT
#define RT_TIMER_THREAD_PRIO 1
#define RT_TIMER_THREAD_STACK_SIZE 8192
#define RT_DEBUG

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "uart0"
#define RT_VER_NUM 0x30103

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 8192
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 220
#define FINSH_THREAD_STACK_SIZE 8192
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 2
#define DFS_FILESYSTEM_TYPES_MAX 2
#define DFS_FD_MAX 16
#define RT_USING_DFS_DEVFS

/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SYSTEM_WORKQUEUE
#define RT_SYSTEM_WORKQUEUE_STACKSIZE 2048
#define RT_SYSTEM_WORKQUEUE_PRIORITY 66
#define RT_USING_SERIAL
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_PIN
#define RT_USING_ADC
#define RT_USING_PWM
#define RT_USING_RTC
#define RTC_SYNC_USING_NTP
#define RTC_NTP_FIRST_SYNC_DELAY 30
#define RTC_NTP_SYNC_PERIOD 3600

/* Using WiFi */


/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC
#define RT_USING_POSIX

/* Network */

/* Socket abstraction layer */

#define RT_USING_SAL

/* protocol stack implement */

#define SAL_USING_LWIP
#define SAL_USING_TLS
#define SAL_USING_POSIX

/* Network interface device */

#define RT_USING_NETDEV
#define NETDEV_USING_IFCONFIG
#define NETDEV_USING_PING
#define NETDEV_USING_NETSTAT
#define NETDEV_IPV6 1
#define NETDEV_IPV4 1

/* light weight TCP/IP stack */

#define RT_USING_LWIP
#define RT_USING_LWIP_MT2625
#define RT_LWIP_IGMP
#define RT_LWIP_ICMP
#define RT_LWIP_DNS

/* Static IPv4 Address */

#define RT_LWIP_IPADDR "192.168.1.30"
#define RT_LWIP_GWADDR "192.168.1.1"
#define RT_LWIP_MSKADDR "255.255.255.0"
#define RT_LWIP_UDP
#define RT_LWIP_TCP
#define RT_LWIP_RAW
#define RT_MEMP_NUM_NETCONN 8
#define RT_LWIP_PBUF_NUM 16
#define RT_LWIP_RAW_PCB_NUM 4
#define RT_LWIP_UDP_PCB_NUM 4
#define RT_LWIP_TCP_PCB_NUM 4
#define RT_LWIP_TCP_SEG_NUM 40
#define RT_LWIP_TCP_SND_BUF 8196
#define RT_LWIP_TCP_WND 8196
#define RT_LWIP_TCPTHREAD_PRIORITY 10
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 8
#define RT_LWIP_TCPTHREAD_STACKSIZE 1024
#define RT_LWIP_ETHTHREAD_PRIORITY 12
#define RT_LWIP_ETHTHREAD_STACKSIZE 1024
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
#define RT_LWIP_USING_PING

/* Modbus master and slave stack */


/* AT commands */


/* VBUS(Virtual Software BUS) */


/* Utilities */

#define RT_USING_ULOG
#define ULOG_OUTPUT_LVL_D
#define ULOG_OUTPUT_LVL 7
#define ULOG_USING_ISR_LOG
#define ULOG_ASSERT_ENABLE
#define ULOG_LINE_BUF_SIZE 256

/* log format */

#define ULOG_OUTPUT_FLOAT
#define ULOG_USING_COLOR
#define ULOG_OUTPUT_TIME
#define ULOG_OUTPUT_LEVEL
#define ULOG_OUTPUT_TAG
#define ULOG_BACKEND_USING_CONSOLE

/* RT-Thread online packages */

/* IoT - internet of things */

#define PKG_USING_PAHOMQTT
#define PAHOMQTT_PIPE_MODE
#define PKG_USING_PAHOMQTT_EXAMPLE
#define PKG_USING_PAHOMQTT_TEST
#define MQTT_USING_TLS
#define RT_PKG_MQTT_THREAD_STACK_SIZE 8192
#define PKG_PAHOMQTT_SUBSCRIBE_HANDLERS 1
#define MQTT_DEBUG
#define PKG_USING_PAHOMQTT_LATEST
#define PKG_USING_WEBCLIENT
#define WEBCLIENT_DEBUG
#define WEBCLIENT_USING_SAMPLES
#define WEBCLIENT_USING_SAL_TLS
#define PKG_USING_WEBCLIENT_V210

/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */

#define PKG_USING_NETUTILS
#define PKG_NETUTILS_IPERF
#define PKG_NETUTILS_NTP
#define NETUTILS_NTP_TIMEZONE 8
#define NETUTILS_NTP_HOSTNAME "cn.ntp.org.cn"
#define NETUTILS_NTP_HOSTNAME2 "ntp.rt-thread.org"
#define NETUTILS_NTP_HOSTNAME3 "edu.ntp.org.cn"
#define PKG_USING_NETUTILS_V110

/* IoT Cloud */


/* security packages */

#define PKG_USING_MBEDTLS

/* Select Root Certificate */

#define PKG_USING_MBEDTLS_DIGICERT_ROOT_CA
#define MBEDTLS_AES_ROM_TABLES
#define MBEDTLS_ECP_WINDOW_SIZE 2
#define MBEDTLS_SSL_MAX_CONTENT_LEN 3584
#define PKG_USING_MBEDTLS_EXAMPLE
#define MBEDTLS_MPI_MAX_SIZE 1024
#define MBEDTLS_CTR_DRBG_KEYSIZE 32
#define PKG_USING_MBEDTLS_V2710

/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */


/* peripheral libraries and drivers */


/* miscellaneous packages */


/* samples: kernel and components samples */


/* Hardware Drivers Config */

#define MTK_MT2526DA

/* Onboard Peripheral Drivers */

#define BSP_USING_CONSOLE_USART
#define NB_PWK_LONGPRESS_SHUTDOWN_ENABLE

/* On-chip Peripheral Drivers */

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_UART0
#define BSP_USING_UART1
#define BSP_USING_ADC
#define BSP_USING_ADC0
#define BSP_USING_PWM
#define BSP_USING_PWM3
#define BSP_USING_PWM3_CLOCK 1
#define BSP_USING_ONCHIP_RTC
#define BSP_USING_SLEEP_MANAGER

/* MT2625 components */

#define NB_USING_LWIP
#define NB_USING_ONENET
#define NB_ONENET_ADDRESS "183.230.40.40"
#define NB_ONENET_PORT "5683"
#define NB_ONENET_LIFE_TIME 3000
#define NB_ONENET_OBJECT_MAX_NUM 10
#define NB_ONENET_INSTANCE_MAX_NUM 10
#define NB_ONENET_USING_LOG
#define NB_USING_RSFOTA
#define NB_USING_TINYDTLS
#define NB_USING_LIBCOAP
#define NB_USING_LWM2M
#define NB_USING_CJSON
#define NB_USING_USB
#define NB_USING_NB_MODEM
#define NB_USING_TEL_CONN_MGR
#define NB_USING_APB_PROXY
#define NB_USING_RIL
#define NB_USING_NIDD
#define NB_USING_SIMAT_PROXY
#define NB_USING_HDK
#define NB_USING_MUX_AP
#define NB_USING_KERNEL_SERVICE
#define NB_USING_NVDM
#define NB_USING_NVDM_MODEM
#define NB_USING_MTK_PORT_SERVICE
#define NB_USING_SOFTWARE_AT_CMD
#define NB_SOFTWARE_AT_CMD_BUF_SIZE 128

/* DNS config */

#define RT_NB_FIRST_DNS "223.5.5.5"
#define RT_NB_SECOND_DNS "114.114.114.114"
#define SW_VER_NUM 0x00010005
#define RT_USING_NB_SAMPLES
#define NB_GPIO_DEMO
#define NB_ADC_DEMO
#define NB_PWM_DEMO
#define NB_UART_DEMO
#define NB_TCP_DEMO
#define NB_UDP_DEMO
#define NB_ONENET_DEMO
#define NB_ONENET_OTA_DEMO

/* sdk config */

#define RT_NB_SDK_VERSION "v1.2.0"
#define RT_NB_DBG_FEATURE
#define PRJ_USING_NBIOT_DEMO

#endif
