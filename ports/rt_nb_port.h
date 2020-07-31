/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-24     MurphyZhao   First version
 */

#ifndef __RT_NB_PORT_H__
#define __RT_NB_PORT_H__

#include <stdint.h>
#include <rtthread.h>

#define RT_NB_IMEI_LEN     (16u) // default 15
#define RT_NB_SN_LEN       (24u) // default 20
#define RT_NB_DEFAULT_LEN  (32u)
#define RT_NB_MAX_APN_NAME (32u)

#define RT_NB_RIL_CMD_OK            (0)
#define RT_NB_RIL_CMD_GENERAL_ERROR (-1)
#define RT_NB_MAX_IPADDR_LEN        (65u) // 64 + 1

typedef struct 
{
    char *buf;
    uint32_t len;
} rt_ext_bytes_t;

typedef struct rt_nb_cfg
{
    char sn[RT_NB_SN_LEN];          /* AT+CGSN */
    char imei[RT_NB_IMEI_LEN];      /* AT+CGSN */
    char imeisv[RT_NB_DEFAULT_LEN]; /* AT+CGSN */
    char svn[RT_NB_DEFAULT_LEN];    /* AT+CGSN */
    char imsi[RT_NB_DEFAULT_LEN];   /* AT+CIMI */
    char iccid[RT_NB_DEFAULT_LEN];  /* AT*MICCID */ /* default size 20 */
} rt_nb_cfg_t;

/**
 * @brief This defines data structure for AT response of "+CESQ".
 */
typedef struct {
    int32_t rxlev;  /**< The received signal strength level. */
    int32_t ber;   /**< The channel bit error rate. */
    int32_t rscp;  /**< The received signal code power. */
    int32_t ecno;  /**< The ratio of the received energy per PN chip to the total received power spectral density. */
    int32_t rsrq;  /**< The reference signal received quality. */
    int32_t rsrp;  /**< The reference signal received power. */
} rt_nb_signal_quality_rsp_t;

/**
 * @brief This defines data structure for AT response of "+CEREG".
 */
typedef struct {
    int32_t n;    /**< Display mode cocnfiguration when "+CEREG" unsolicited result code is received. */
    int32_t stat;    /**< Indicates EPS registration status. */
    int32_t tac;    /**< Two bytes tracking area code in hexadecimal format. */
    int32_t ci;    /**< String type, four byte GERAN/UTRAN/E-TRAN cell id in hexadecimal format. */
    int32_t act;    /**< Access technology of the registered network. */
    int32_t cause_type;    /**< Integer type, indicates the type of the field "reject cause". */
    int32_t reject_cause;    /**< Integer type, contains the cause of the failed registration. */
    int32_t active_time;    /**< One byte in an 8 bit format, indicates the Active Time value allocated to the UE in E-UTRAN. */
    int32_t periodic_tau;    /**< One byte in an 8 bit format, indicates the extended periodic TAU value allocated to the UE in E-UTRAN. */
} rt_nb_network_registration_status_rsp_t;

rt_err_t rt_nb_ril_init(void);

int rt_nb_cfg_sync(void);
rt_nb_cfg_t *rt_nb_cfg_get(void);

char *rt_nb_manufacturer_id_get(void);
char *rt_nb_model_id_get(void);
char *rt_nb_sw_ver_get(void);
char *rt_nb_hw_ver_get(void);

char *rt_nb_imei_get(void);
char *rt_nb_imsi_get(void);
char *rt_nb_iccid_get(void);
char *rt_nb_sn_get(void);

/**
 * rt_nb_get_csq
 * 
 * @brief Query nbiot system`s rssi and ber(bit error rate)
 * 
 * @param rssi  Input param. The received signal strength indication.
 * @param ber   Input param. The bit error rate.
 * 
 * @return rt_err_t type
 * 
*/
rt_err_t rt_nb_csq_get(int32_t *rssi, int32_t *ber);

rt_err_t rt_nb_cesq_get(rt_nb_signal_quality_rsp_t *signal);

/**
 * rt_nb_cfun_set_get
 * 
 * @brief set or get cfun
 * 
 * @param *status   When direction is 1, *status as input param;
 *                  When direction is 0, *status as output param.
 * @param direction
 *          0: get
 *          1: set
*/
rt_err_t rt_nb_cfun_set_get(int32_t *status, uint8_t direction);

rt_err_t rt_nb_cereg_get(rt_nb_network_registration_status_rsp_t  *cereg);

/**
 * rt_nb_cereg_set
 * 
 * @brief AT+CEREG=<n>: Set the type of status returned
 * 
 * @param n
 *        - 0 or 1: <n>,<stat>
 *        - 2:<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>]] 
 *        - 3:<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>],[<cause_type>,<reject_cause>]] 
 *        - 4:<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>][,,[,[<Active-Time>],[<Periodic-RAU>],[<GPRS-READY-timer>]]]]
 *        - 5:<n>,<stat>[,[<tac>],[<ci>],[<act>],[<rac>][,[<cause_type>],[<reject_cause>][,[<Active-Time>],[<Periodic-TAU>]]]]]
 * @return rt_err_t type
*/
rt_err_t rt_nb_cereg_set(uint8_t n);

/**
 * rt_nb_sim_card_status_get
 * 
 * @brief check sim card status
 * @param *status, input param
 *          1: ready
 *          0: not ready
 * @return rt_err_t type
*/
rt_err_t rt_nb_sim_card_status_get(uint8_t *status);

rt_err_t rt_nb_apn_get(char *apn_name, uint8_t len);
rt_err_t rt_nb_apn_set(char *pdp_type, char *apn, char *username, char *password);

rt_err_t rt_nb_edrx_set(int32_t status, uint8_t *active_time);
rt_err_t rt_nb_psm_set(int32_t status, uint8_t *tau, uint8_t *active_time);

rt_err_t rt_nb_cgpaddr_get(char *ip_addr, uint8_t len);
rt_err_t rt_nb_cclk_get(char *cclk, uint8_t len);

void nbiot_sdk_version_get(char *buf, int len);
#endif /* __RT_NB_PORT_H__ */
