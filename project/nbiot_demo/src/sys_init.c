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

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"

#include "hal.h"
#include "memory_attribute.h"
#include "bsp_gpio_ept_config.h"
#include "nvdm.h"
#include "hal_rtc_external.h"
#include "hal_rtc_internal.h"
#include "hal_sleep_manager_internal.h"
#include "nb_custom_port_config.h"
#include "gkimisc.h"

/* device.h includes */
#include "hal_platform.h"
#include "rtthread.h"
#include "mt2625.h"

#ifdef MTK_NVDM_MODEM_ENABLE
#include "nvdm_modem.h"
#endif

#ifdef MTK_NB_MODEM_ENABLE
extern bool N1RfShouldSetPinmux(void);
#endif

#ifdef MTK_SYSTEM_CLOCK_SET
static const uint32_t target_freq = MTK_SYSTEM_CLOCK_SET;
#else
/* target system frequency selection */
#ifdef MTK_SYSTEM_CLOCK_156M
static const uint32_t target_freq = 156000;
#elif defined(MTK_SYSTEM_CLOCK_78M)
static const uint32_t target_freq = 78000;
#elif defined(MTK_SYSTEM_CLOCK_26M)
static const uint32_t target_freq = 26000;
#else
static const uint32_t target_freq = 104000;
#endif
#endif

#ifdef HAL_SLEEP_MANAGER_ENABLED
extern uint8_t sys_lock_handle;
#endif

/* Private functions -----------------------------------------------------------*/
#ifdef __GNUC__
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the HAL_UART_0 one at a time */

    /* Printf to UART 0 until OS is ready to printf to UART 1 */
    if (KiOsSystemIsInitialised()) {
        hal_uart_put_char(HAL_UART_1, ch);
    } else {
        hal_uart_put_char(HAL_UART_0, ch);
    }

    return ch;
}

/**
*@brief Configure and initialize UART hardware initialization for logging.
*@param None.
*@return None.
*/

bool use_slow_bitfile = 0;

static void plain_log_uart_init(void)
{
#if !defined(MTK_PORT_SERVICE_ENABLE) 
    hal_uart_config_t uart_config;

    /* COM port settings */
    /* AT commands to 115200 */
    uart_config.baudrate = HAL_UART_BAUDRATE_115200;
    uart_config.word_length = HAL_UART_WORD_LENGTH_8;
    uart_config.stop_bit = HAL_UART_STOP_BIT_1;
    uart_config.parity = HAL_UART_PARITY_NONE;
    hal_uart_init(HAL_UART_0, &uart_config);
    /* GKI and HSL traces to max */
    uart_config.baudrate = HAL_UART_BAUDRATE_921600;

    hal_uart_init(HAL_UART_1, &uart_config);
    hal_uart_init(HAL_UART_2, &uart_config);
#endif
}


void SystemClock_Config(void)
{
    hal_clock_init();
}

static void cache_init(void)
{
    hal_cache_region_t region, region_number;

    /* Max region number is 16 */
    hal_cache_region_config_t region_cfg_tbl[] = {
        /* cacheable address, cacheable size(both MUST be 4k bytes aligned) */
        /* Flash memory */
        {RTOS_BASE, RTOS_LENGTH},
        /* virtual memory for AP */
        {VRAM_BASE, VRAM_LENGTH + VRAM_MD_LENGTH},
        /* RTC SRAM */
        {RETSRAM_BASE, RETSRAM_LENGTH + RETSRAM_MD_LENGTH}
    };

    region_number = (hal_cache_region_t)(sizeof(region_cfg_tbl) / sizeof(region_cfg_tbl[0]));

    hal_cache_init();
    hal_cache_set_size(HAL_CACHE_SIZE_32KB);
    for (region = HAL_CACHE_REGION_0; region < region_number; region++) {
        hal_cache_region_config(region, &region_cfg_tbl[region]);
        hal_cache_region_enable(region);
    }
    for (; region < HAL_CACHE_REGION_MAX; region++) {
        hal_cache_region_disable(region);
    }
    hal_cache_enable();
}

#ifdef HAL_MPU_MODULE_ENABLED
/**
* @brief       caculate actual bit value of region size.
* @param[in]   region_size: actual region size.
* @return      corresponding bit value of region size for MPU setting.
*/
static uint32_t caculate_mpu_region_size(uint32_t region_size)
{
    uint32_t count;

    if (region_size < 32) {
        return 0;
    }
    for (count = 0; ((region_size  & 0x80000000) == 0); count++, region_size  <<= 1);
    return 30 - count;
}

/**
* @brief       This function is to initialize MPU.
* @param[in]   None.
* @return      None.
*/
static void mpu_init(void)
{
    hal_mpu_region_t region, region_number;
    hal_mpu_region_config_t region_config;
    typedef struct {
        uint32_t mpu_region_base_address;/**< MPU region start address */
        uint32_t mpu_region_end_address;/**< MPU region end address */
        hal_mpu_access_permission_t mpu_region_access_permission; /**< MPU region access permission */
        uint8_t mpu_subregion_mask; /**< MPU sub region mask*/
        bool mpu_xn;/**< XN attribute of MPU, if set TRUE, execution of an instruction fetched from the corresponding region is not permitted */
    } mpu_region_information_t;

    //VRAM: CODE+RO DATA for AP
    extern uint32_t Image$$RAM_TEXT$$Base;
    extern uint32_t Image$$RAM_TEXT$$Limit;

    //VRAM: CODE+RO DATA for AP
    extern uint32_t Image$$MD_CACHED_RAM_TEXT$$Base;
    extern uint32_t Image$$MD_CACHED_RAM_TEXT$$Limit;

    //TCM: CODE+RO DATA for AP
    extern uint32_t Image$$TCM$$RO$$Base;
    extern uint32_t Image$$TCM$$RO$$Limit;

    //TCM: CODE+RO DATA for MD
    extern uint32_t Image$$MD_TCM$$RO$$Base;
    extern uint32_t Image$$MD_TCM$$RO$$Limit;

    //STACK END
    extern unsigned int Image$$STACK$$ZI$$Base;

    /* MAX region number is 8 */
    mpu_region_information_t region_information[] = {
        /* mpu_region_start_address, mpu_region_end_address, mpu_region_access_permission, mpu_subregion_mask, mpu_xn */
        {(uint32_t) &Image$$RAM_TEXT$$Base, (uint32_t) &Image$$RAM_TEXT$$Limit, HAL_MPU_READONLY, 0x0, FALSE}, //Virtual memory for AP
        {(uint32_t) &Image$$RAM_TEXT$$Base - VRAM_BASE, (uint32_t) &Image$$RAM_TEXT$$Limit - VRAM_BASE, HAL_MPU_NO_ACCESS, 0x0, TRUE}, //RAM code+RAM rodata for AP
        {(uint32_t) &Image$$MD_CACHED_RAM_TEXT$$Base, (uint32_t) &Image$$MD_CACHED_RAM_TEXT$$Limit, HAL_MPU_READONLY, 0x0, FALSE}, //Virtual memory for MD
        {(uint32_t) &Image$$MD_CACHED_RAM_TEXT$$Base - VRAM_BASE, (uint32_t) &Image$$MD_CACHED_RAM_TEXT$$Limit - VRAM_BASE, HAL_MPU_NO_ACCESS, 0x0, TRUE}, //RAM code+RAM rodata for MD

        {(uint32_t) &Image$$TCM$$RO$$Base, (uint32_t) &Image$$TCM$$RO$$Limit, HAL_MPU_READONLY, 0x0, FALSE},//TCM code+TCM rodata for MD
        {(uint32_t) &Image$$MD_TCM$$RO$$Base, (uint32_t) &Image$$MD_TCM$$RO$$Limit, HAL_MPU_READONLY, 0x0, FALSE},//MD TCM code + MD TCM rodata
        {(uint32_t) &Image$$STACK$$ZI$$Base, (uint32_t) &Image$$STACK$$ZI$$Base + 32, HAL_MPU_READONLY, 0x0, TRUE}, //Stack end check for stack overflow
    };

    hal_mpu_config_t mpu_config = {
        /* PRIVDEFENA, HFNMIENA */
        TRUE, TRUE
    };

    region_number = (hal_mpu_region_t)(sizeof(region_information) / sizeof(region_information[0]));

    hal_mpu_init(&mpu_config);
    for (region = HAL_MPU_REGION_0; region < region_number; region++) {
        /* Updata region information to be configured */
        region_config.mpu_region_address = region_information[region].mpu_region_base_address;
        region_config.mpu_region_size = (hal_mpu_region_size_t) caculate_mpu_region_size(region_information[region].mpu_region_end_address - region_information[region].mpu_region_base_address);
        region_config.mpu_region_access_permission = region_information[region].mpu_region_access_permission;
        region_config.mpu_subregion_mask = region_information[region].mpu_subregion_mask;
        region_config.mpu_xn = region_information[region].mpu_xn;

        if (hal_mpu_region_configure(region, &region_config) != HAL_MPU_STATUS_OK) {
            hal_mpu_region_disable(region);
        } else {
            hal_mpu_region_enable(region);
        }
    }
    /* make sure unused regions are disabled */
    for (; region < HAL_MPU_REGION_MAX; region++) {
        hal_mpu_region_disable(region);
    }
    hal_mpu_enable();
}
#endif

#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
#include "hal_wdt.h"

void system_wdt_occur(hal_wdt_reset_status_t mode)
{
   if(HAL_WDT_TIMEOUT_RESET == mode)
      DevFail("Watchdog timeout");
   else
      DevFail("Invalid SW watchdog");
}

/* start watchdog, timeout is 30s. Feed it at vApplicationIdleHook
   when watchdog timeout occurs, WDT_ISR occurs, then invoke assert function */
void system_start_wdt(void)
{
    hal_wdt_config_t wdt_config;
    wdt_config.mode = HAL_WDT_MODE_RESET;
    wdt_config.seconds = SYSTEM_HANG_CHECK_TIMEOUT_DURATION;

    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
    hal_wdt_init(&wdt_config);
    hal_wdt_register_callback(system_wdt_occur);
    hal_wdt_enable(HAL_WDT_ENABLE_MAGIC);
}
#endif

static void prvSetupHardware(void)
{
#ifndef NB_PWK_LONGPRESS_SHUTDOWN_ENABLE
    pwkey_long_press_shutdown_ctrl(0);
#endif
    /* System HW initialization */
#ifdef HAL_SLEEP_MANAGER_ENABLED
    hal_sleep_manager_init();
#endif

    hal_dvfs_init();
    hal_dvfs_target_cpu_frequency(target_freq, HAL_DVFS_FREQ_RELATION_L);

    cache_init();

#ifdef HAL_MPU_MODULE_ENABLED
    mpu_init();
#endif

    /* Peripherals initialization */

    /* Set SYS region to Bufferable mode. TODO MACRO! */
	  *(int*)0xE0100014 = 0x36;

    plain_log_uart_init();

#ifdef HAL_FLASH_MODULE_ENABLED
    hal_flash_init();
#endif
    hal_nvic_init();
    NVIC_SetPriority(SVC_IRQn, 0xFF);

    /* Board HW initialization */

    hal_dcxo_init();

/* wdt is enabled in bootloader, if infinite loop occurs before this line,
   wdt reset will occurs. re-config wdt here for system hang check */
#ifdef MTK_SYSTEM_HANG_CHECK_ENABLE
    //enable wdt
    system_start_wdt();
#else
    hal_wdt_disable(HAL_WDT_DISABLE_MAGIC);
#endif
}

#ifdef HAL_RTC_FEATURE_RETENTION_SRAM
static uint32_t rtc_retention_sram_caculate_used_bank(uint32_t used_size)
{
    uint32_t per_bank_size = RETSRAM_PER_BANK_LENGTH, used_bank;

    used_bank = ((used_size + (per_bank_size-1)) / per_bank_size);

    return ((1 << used_bank) - 1);
}

static void rtc_retention_sram_power_saving_check(void)
{
    extern uint32_t Image$$RETSRAM_DATA$$RW$$Base;
    extern uint32_t Image$$RETSRAM_DATA$$ZI$$Limit;
    extern uint32_t Image$$MD_RETSRAM_DATA$$ZI$$Limit;
    unsigned int used_banks, ap_used_banks, ap_reserved_banks;

    ap_used_banks = rtc_retention_sram_caculate_used_bank((uint32_t)&Image$$RETSRAM_DATA$$ZI$$Limit - (uint32_t)&Image$$RETSRAM_DATA$$RW$$Base);
    ap_reserved_banks = (RETSRAM_LENGTH / RETSRAM_PER_BANK_LENGTH);
    ap_reserved_banks = (1 << ap_reserved_banks) - 1;

    used_banks = rtc_retention_sram_caculate_used_bank((uint32_t)&Image$$MD_RETSRAM_DATA$$ZI$$Limit - (uint32_t)&Image$$RETSRAM_DATA$$RW$$Base);
    used_banks &= ~ap_reserved_banks;
    used_banks |= ap_used_banks;

    printf("used_banks=%x, ap_used=%x, ap_reserved=%x\r\n",
           used_banks, ap_used_banks, ap_reserved_banks);

    hal_rtc_retention_sram_config(((~used_banks)&HAL_RTC_RETENTION_SRAM_NUMBER_MASK), HAL_RTC_SRAM_PD_MODE);
}
#endif

void rtc_gpio_callback(void *data, uint32_t mode)
{
    hal_rtc_gpio_control_t gpio_control;
    // Control RTC GPIO 0 output high by software.
    gpio_control.is_sw_control = true;
    gpio_control.is_sw_output_high = false;
    hal_rtc_configure_gpio(HAL_RTC_GPIO_0, &gpio_control);
}

static void rtc_gpio_init(void)
{
    hal_rtc_gpio_control_t gpio_control;
    // Control RTC GPIO 0 output high by software.
    gpio_control.is_sw_control = true;
    gpio_control.is_sw_output_high = true;
    hal_rtc_configure_gpio(HAL_RTC_GPIO_0, &gpio_control);

    hal_sleep_manager_register_suspend_callback(rtc_gpio_callback, NULL, HAL_SLEEP_MODE_DEEP_SLEEP | HAL_SLEEP_MODE_DEEPER_SLEEP);
}

#ifdef HAL_SLEEP_MANAGER_ENABLED
void system_default_sleep_lock(void)
{
    nvdm_status_t status;
    uint32_t sys_default_lock = false;
    uint32_t len = sizeof(sys_default_lock);

    status = nvdm_read_data_item("sleep_manager", "sys_default_lock", 
            (uint8_t *)&sys_default_lock, &len);
    if (status != NVDM_STATUS_OK)
    {
        rt_kprintf("read sys_default_lock failed, status = %d\n", status);
        len = sizeof(sys_default_lock);
        nvdm_write_data_item("sleep_manager", "sys_default_lock", 
                NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *)&sys_default_lock, len);
    }

    if (sys_lock_handle == 0xFF)
    {
        sys_lock_handle = hal_sleep_manager_set_sleep_handle("sys_init");
    }

    if (sys_default_lock == true)
    {
        hal_sleep_manager_acquire_sleeplock(sys_lock_handle, HAL_SLEEP_LOCK_ALL);
    }
}
#endif

/**
* @brief       This function is to do system initialization, eg: system clock, hardware and logging port.
* @param[in]   None.
* @return      None.
*/
int system_init(void)
{
    rtc_power_on_result_t result;
    /* Configure system clock */
    // SystemClock_Config();    /* move to drv_common.c to init */
    // SystemCoreClockUpdate(); /* move to drv_common.c to init */

    /* Configure the hardware */
    prvSetupHardware();

    /*ept init*/
    bsp_ept_gpio_setting_init();

#ifdef MTK_NB_MODEM_ENABLE
    /* Apply to MT2625 E2 EVB pinmux */
    if (true == N1RfShouldSetPinmux()) {
        /* Set GPIO_4 KP_ROW1 */
        hal_pinmux_set_function(HAL_GPIO_4, 4);
        /* Set GPIO_32 UART2_TXD */
        hal_pinmux_set_function(HAL_GPIO_32, 3);
        /* Disbale pull */
        hal_gpio_disable_pull(HAL_GPIO_32);
    }
#endif

    /* Enable NVDM feature */
#ifdef MTK_NVDM_ENABLE
    if (NVDM_STATUS_OK == nvdm_init())
    {
#ifdef RT_NB_DBG_FEATURE
        rt_kprintf("nvdm init pass\n");
#endif
        serial_port_dev_t serial_dev = SERIAL_PORT_DEV_UNDEFINED;
        if (serial_port_config_read_dev_number("connl", &serial_dev) == SERIAL_PORT_STATUS_OK)
        {
            if (serial_dev != DEFAULT_PORT_FOR_AT_CMD)
            {
                serial_dev = DEFAULT_PORT_FOR_AT_CMD;
                serial_port_config_write_dev_number("connl", serial_dev);
            }
        }

        if (serial_port_config_read_dev_number("emmi", &serial_dev) == SERIAL_PORT_STATUS_OK)
        {
            if (serial_dev != DEFAULT_PORT_FOR_EMMI_TRACE)
            {
                serial_dev = DEFAULT_PORT_FOR_EMMI_TRACE;
                serial_port_config_write_dev_number("emmi", serial_dev);
            }
        }

        if (serial_port_config_read_dev_number("uls", &serial_dev) == SERIAL_PORT_STATUS_OK)
        {
            if (serial_dev != DEFAULT_PORT_FOR_ULS_TRACE)
            {
                serial_dev = DEFAULT_PORT_FOR_ULS_TRACE;
                serial_port_config_write_dev_number("uls", serial_dev);
            }
        }
#ifdef RT_NB_DBG_FEATURE
        rt_kprintf("uart reconfig pass\n");
#endif
    }
#endif

#ifdef MTK_NVDM_MODEM_ENABLE
    /*modem nvdm init */
    nvdm_modem_init();
    rt_kprintf("nvdm modem init pass\n");
#endif

    hal_dvfs_target_cpu_frequency(26000, HAL_DVFS_FREQ_RELATION_L);
    hal_dcxo_load_calibration();
    hal_dvfs_target_cpu_frequency(target_freq, HAL_DVFS_FREQ_RELATION_L);
#ifdef RT_NB_DBG_FEATURE
    rt_kprintf("retarget freq pass\n");
#endif

#ifdef HAL_RTC_MODULE_ENABLED
    result = rtc_power_on_result_external();
    rt_kprintf("power on from <%d> mode\n", result);

#if defined(HAL_RTC_FEATURE_RETENTION_SRAM) && defined(HAL_SLEEP_MANAGER_ENABLED)
    rtc_retention_sram_power_saving_check();

    /* restore rtcram when bootfrom deeper sleep mode */
    if (DEEPER_SLEEP == result)
    {
        rt_kprintf("power on from deepsleep\n");
        sleep_managerment_restore_rtcram();
        hal_cache_invalidate_all_cache_lines();
    }

#ifdef MTK_NVDM_ENABLE
    system_default_sleep_lock();
#ifdef RT_NB_DBG_FEATURE
    rt_kprintf("default sleep lock pass\n");
#endif
#endif /* MTK_NVDM_ENABLE */

#endif /* HAL_RTC_FEATURE_RETENTION_SRAM && HAL_SLEEP_MANAGER_ENABLED */

    rtc_init_phase_2();
    rtc_gpio_init();

#endif /* HAL_RTC_MODULE_ENABLED */

    rt_kprintf("system initialize done@%d.\r\n", (int)hal_dvfs_get_cpu_frequency());

    return 0;
}
