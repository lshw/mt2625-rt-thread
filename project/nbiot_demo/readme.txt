/* Copyright Statement:
 *
 * (C) 2005-2016  MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. ("MediaTek") and/or its
 * licensors. Without the prior written permission of MediaTek and/or its
 * licensors, any reproduction, modification, use or disclosure of MediaTek
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute
 * (as applicable) MediaTek Software if you have agreed to and been bound by
 * the applicable license agreement with MediaTek ("License Agreement") and
 * been granted explicit permission to do so within the License Agreement
 * ("Permitted User").  If you are not a Permitted User, please cease any
 * access or use of MediaTek Software immediately.
 */

/**
  * @page freertos thread creation example project
  * @{

@par Overview
  - Example description
    - This example describes how to setup FreeRTOS multitasking environment.
      The project creates four tasks. Each task sleeps for a certain amount
      of time measured in OS ticks and prints the logs when it awakes.
  - Process / procedure of the example project
    - Declare and initialize four tasks and schedule at different intervals.
  - Results
    - Log will show when a task wakes up.

@par Hardware and software environment
  - Supported HDK
    - 2625 FPGA.
  - HDK switches and pin configuration
    - N/A.    
  - Environment configuration
    - Please refer to FPGA & IVP load user guide











   - A serial tool is required, such as hyper terminal, for UART logging.
   - COM port settings. baudrate: 115200, data bits: 8, stop bit: 1, parity: none and flow control: off.

@par Directory contents
  - Source and header files
   - freertos_2625/src/main.c                            Main program.
   - freertos_2625/src/system_mt2625.c                   The configuration file of the Cortex-M4 with floating point core registers and system clock.
   - freertos_2625/src/md_init.c                         This file is for Modem init.
   - freertos_2625/inc/FreeRTOSConfig.h                  FreeRTOS feature configuration file. 
   - freertos_2625/inc/hal_feature_config.h              MT2625's feature configuration file.
   - freertos_2625/inc/memory_map.h                      MT2625's memory layout symbol file.
   - freertos_2625/inc/cust_charging.h                   System clock configuration.
   - freertos_2625/GCC/startup_mt2625.s                  MT2625's startup file for GCC.
   - freertos_2625/GCC/startup_md2625.s                  MT2625 MD's startup file for GCC.
   - freertos_2625/GCC/syscalls.c                        The minimal implementation of the system calls.
  - Project configuration files using GCC
   - freertos_2625/GCC/feature.mk		         Feature configuration.
   - freertos_2625/GCC/Makefile                          Makefile.    
   - freertos_2625/GCC/tcm.ld                            Linker script. 
   - freertos_2625/GCC/ram.ld                            Linker script.
  - Project configuration files using Keil
   - N/A   
@par Run the demo
  - Build the example project with a command "./build.sh mt2625_evb
    freertos_2625" from the SDK root folder and download the .elf
    file to 2625 FPGA with coretrace.
  - Connect the PC and FPGA UART-0 with UART cable.
  - COM port settings. baudrate: 115200, data: 8 bits, stop bit: 1, parity:
      none and flow control: off.
  - Run the example. The log will show "hello world x", where x is the task
    ID with the range from 0 to 3. And this result indicates a successful
    operation.
*/
/**
 * @}
 * @}
 * @}
 */
*/