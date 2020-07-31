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

.file "startup_mt2625.s"
.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

.equ    WDT_Base,           0xA2090000
.equ    WDT_Disable,        0x11
.equ    Remap_Base,         0xE0181000
.equ    Remap_Entry_HI0,    0x10000023
.equ    Remap_Entry_LO0,    0x0

.global  g_pfnVectors
.global  Default_Handler

/**
 * @brief  reset_handler is the entry point that processor starts to boot
 * @param  None
 * @retval : None
*/

  .section  .reset_handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:
/* set stack pointer */
  ldr  sp, =_stack_end

/* interrupt disable */
  cpsid i

/* preinit cache to accelerate region init progress */ 
  bl CachePreInit

/* watch dog disable */
  ldr  r0, =WDT_Base
  ldr  r1, =WDT_Disable
  str  r1, [r0, #0]


/* make virtual space available */

  ldr  r0, =Remap_Entry_HI0
  ldr  r1, =Remap_Entry_LO0
  ldr  r2, =Remap_Base
  str  r0, [r2], #4
  str  r1, [r2, #0]

/* stack space zero init */
  movs  r0, #0
  ldr  r1, =_stack_start
  ldr  r2, =_stack_end
FillZero:
  str  r0, [r1], #4
  cmp  r1, r2
  blo  FillZero

/* ram_text section init*/
  ldr  r1, =_ram_code_load
  ldr  r2, =_ram_code_start
  ldr  r3, =_ram_code_end
  bl  Data_Init

/* md_ram_text section init*/
  ldr  r1, =_md_ram_code_load
  ldr  r2, =_md_ram_code_start
  ldr  r3, =_md_ram_code_end
  bl  Data_Init   

/* noncached_data section init*/
  ldr  r1, =_ram_noncached_rw_load
  ldr  r2, =_ram_noncached_rw_start
  ldr  r3, =_ram_noncached_rw_end
  bl  Data_Init

/* md_noncached_data section init*/
  ldr  r1, =_md_ram_noncached_rw_load
  ldr  r2, =_md_ram_noncached_rw_start
  ldr  r3, =_md_ram_noncached_rw_end
  bl  Data_Init

/* cached_data section*/
  ldr  r1, =_data_load
  ldr  r2, =_data_start
  ldr  r3, =_data_end
  bl  Data_Init

/* md_cached_data section*/
  ldr  r1, =_md_data_load
  ldr  r2, =_md_data_start
  ldr  r3, =_md_data_end
  bl  Data_Init

/* tcm section init*/
  ldr  r1, =_tcm_text_load
  ldr  r2, =_tcm_text_start
  ldr  r3, =_tcm_text_end
  bl  Data_Init

  ldr  r2, =_tcm_zi_start
  ldr  r3, =_tcm_zi_end
  bl  Bss_Init
  
/* md_tcm section init*/
  ldr  r1, =_md_tcm_text_load
  ldr  r2, =_md_tcm_text_start
  ldr  r3, =_md_tcm_text_end
  bl  Data_Init

  ldr  r2, =_md_tcm_zi_start
  ldr  r3, =_md_tcm_zi_end
  bl  Bss_Init
  
  ldr  r2, =_bss_start
  ldr  r3, =_bss_end
  bl  Bss_Init

  ldr  r2, =_md_bss_start
  ldr  r3, =_md_bss_end
  bl  Bss_Init   
  
  ldr  r2, =_ram_noncached_zi_start
  ldr  r3, =_ram_noncached_zi_end
  bl  Bss_Init

  ldr  r2, =_md_ram_noncached_zi_start
  ldr  r3, =_md_ram_noncached_zi_end
  bl  Bss_Init  

/*sysram_data section init*/
  ldr  r1, = _sysram_data_load
  ldr  r2, = _sysram_data_start
  ldr  r3, = _sysram_data_end
  bl  Data_Init

  ldr  r2, = _sysram_bss_start 
  ldr  r3, = _sysram_bss_end
  bl  Bss_Init
  
 /* if return value is 1 or 2, boot from retention mode and bypass retention memory initialization */ 
  ldr r1, = rtc_power_on_result_external
  blx r1 
  cmp r0, #1
  beq MD_INIT_PHASE_1
  cmp r0, #2
  beq MD_INIT_PHASE_1
  
 /*ap retentin ram section init*/
  ldr  r1, = _retsram_data_load
  ldr  r2, = _retsram_data_start
  ldr  r3, = _retsram_data_end
  bl  Data_Init

  ldr  r2, = _retsram_bss_start 
  ldr  r3, = _retsram_bss_end
  bl  Bss_Init 
   
  /*md retentin ram section init*/
  ldr  r1, = _md_retsram_data_load
  ldr  r2, = _md_retsram_data_start
  ldr  r3, = _md_retsram_data_end
  bl  Data_Init

  ldr  r2, = _md_retsram_bss_start 
  ldr  r3, = _md_retsram_bss_end
  bl  Bss_Init 

MD_INIT_PHASE_1:
/* jump to md_init_phase_1*/  
  bl md_init_phase_1
  
/* Call the clock system intitialization function.*/
  ldr r0, =SystemInit
  blx r0

/* Call the application's entry point.*/
/*  ldr r0, =main */
/*  bx r0 */
  bl entry
  bx  lr
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is data init sub-function
 * @param  None
 * @retval None
*/
  .section  .reset_handler.Data_Init,"ax",%progbits
Data_Init:
CopyDataLoop:
  cmp     r2, r3
  ittt    lo
  ldrlo   r0, [r1], #4
  strlo   r0, [r2], #4
  blo     CopyDataLoop
  bx  lr
  .size  Data_Init, .-Data_Init

/**
 * @brief  This is bss init sub-function
 * @param  None
 * @retval None
*/
  .section  .reset_handler.Bss_Init,"ax",%progbits
Bss_Init:
ZeroBssLoop:
  cmp     r2, r3
  ittt    lo
  movlo   r0, #0
  strlo   r0, [r2], #4
  blo     ZeroBssLoop
  bx  lr
  .size  Bss_Init, .-Bss_Init

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None
 * @retval None
*/
  .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
  cpsid i
  bl Hard_Fault_Handler
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler

/**
 * @brief  This is the code for system exception handling(svcCall,systick,pendSV), must suspend 
           flash before fetch code from flash
 * @param  None
 * @retval None
*/

  .section  .tcm_code,"ax",%progbits
  .type  SvcCall_Main, %function
  .global  SvcCall_Main
SvcCall_Main:
  cpsid i
  push {lr}
  /*blx Flash_ReturnReady*/
  pop  {lr}
  cpsie i 
  b SVC_Handler
  .size  SvcCall_Main, .-SvcCall_Main

  .type  PendSV_Main, %function
  .global  PendSV_Main
PendSV_Main:
  cpsid i
  push {lr}
 /* blx Flash_ReturnReady*/
  pop  {lr}
  cpsie i 
  b PendSV_Handler
  .size  PendSV_Main, .-PendSV_Main  

  .type  SysTick_Main, %function
  .global  SysTick_Main
SysTick_Main:
  cpsid i
  push {lr}
 /* blx Flash_ReturnReady*/
  cpsie i 
  blx SysTick_Handler
  pop {lr}
  bx lr
  .size  SysTick_Main, .-SysTick_Main    
    
/******************************************************************************
*
* The minimal vector table for a Cortex M4. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
*******************************************************************************/
  .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors


g_pfnVectors:
  .word  _stack_end
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler /* RAW: SvcCall_Main SVC_Handler */
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler /* RAW: PendSV_Main */
  .word  SysTick_Handler /* RAW: SysTick_Main */

/* External Interrupts */
  .word     isrC_main     /*16:  INT 0	MDSYS_IRQ[0]      */
  .word     isrC_main     /*17:  INT 1	MDSYS_IRQ[1]      */
  .word     isrC_main     /*18:  INT 2	MDSYS_IRQ[2]      */
  .word     isrC_main     /*19:  INT 3	MDSYS_IRQ[3]      */
  .word     isrC_main     /*20:  INT 4	MDSYS_IRQ[4]      */
  .word     isrC_main     /*21:  INT 5	MDSYS_IRQ[5]      */
  .word     isrC_main     /*22:  INT 6	MDSYS_IRQ[6]      */
  .word     isrC_main     /*23:  INT 7	MDSYS_IRQ[7]      */
  .word     isrC_main     /*24:  INT 8	MD_Reserved_IRQ_0 */
  .word     isrC_main     /*25:  INT 9	MD_Reserved_IRQ_1 */
  .word     isrC_main     /*26:  INT 10	MD_Reserved_IRQ_2 */
  .word     isrC_main     /*27:  INT 11	MD_Reserved_IRQ_3 */
  .word     isrC_main     /*28:  INT 12	GPIO MIRQ         */
  .word     isrC_main     /*29:  INT 13	OS GPT            */
  .word     isrC_main     /*30:  INT 14	MCU DMA           */
  .word     isrC_main     /*31:  INT 15	Sensor DMA        */
  .word     isrC_main     /*32:  INT 16	SPI master 0      */
  .word     isrC_main     /*33:  INT 17	SPI master 1      */
  .word     isrC_main     /*34:  INT 18	SPI slave         */
  .word     isrC_main     /*35:  INT 19	SDIO slave        */
  .word     isrC_main     /*36:  INT 20	SDIO master       */
  .word     isrC_main     /*37:  INT 21	SDIO master       */
  .word     isrC_main     /*38:  INT 22	SDIO master       */
  .word     isrC_main     /*39:  INT 23	SDIO master       */
  .word     isrC_main     /*40:  INT 24	TRNG              */
  .word     isrC_main     /*41:  INT 25	CRYPTO_ENGINE     */
  .word     isrC_main     /*42:  INT 26	UART0             */
  .word     isrC_main     /*43:  INT 27	UART1             */
  .word     isrC_main     /*44:  INT 28	UART2             */
  .word     isrC_main     /*45:  INT 29	audio_top         */
  .word     isrC_main     /*46:  INT 30	asys_top          */
  .word     isrC_main     /*47:  INT 31	I2C0              */
  .word     isrC_main     /*48:  INT 32	I2C1              */
  .word     isrC_main     /*49:  INT 33	I2C2              */
  .word     isrC_main     /*50:  INT 34	RTC               */
  .word     isrC_main     /*51:  INT 35	GPTIMER           */
  .word     isrC_main     /*52:  INT 36	SPM               */
  .word     isrC_main     /*53:  INT 37	RGU               */
  .word     isrC_main     /*54:  INT 38	PMU_DIG_CORE      */
  .word     isrC_main     /*55:  INT 39	EINT              */
  .word     isrC_main     /*56:  INT 40	SFC               */
  .word     isrC_main     /*57:  INT 41	SENSOR_CTRL_TOP_PD*/
  .word     isrC_main     /*58:  INT 42	SENSOR_CTRL_TOP_AO*/
  .word     isrC_main     /*59:  INT 43	KP                */
  .word     isrC_main     /*60:  INT 44	XTAL_CTL          */
  .word     isrC_main     /*61:  INT 45	USB               */
  .word     isrC_main     /*62:  INT 46	USIM              */
  .word     isrC_main     /*63:  INT 47	CIPHER              */
  .word     isrC_main     /*64:  INT 48	MDSYS_IRQ[8]      */
  .word     isrC_main     /*65:  INT 49	MDSYS_IRQ[9]      */
  .word     isrC_main     /*66:  INT 50	MDSYS_IRQ[10]     */
  .word     isrC_main     /*67:  INT 51	MDSYS_IRQ[11]     */
  .word     isrC_main     /*68:  INT 52	MDSYS_IRQ[12]     */
  .word     isrC_main     /*69:  INT 53	MDSYS_IRQ[13]     */
  .word     isrC_main     /*70:  INT 54	MDSYS_IRQ[14]     */
  .word     isrC_main     /*71:  INT 55	MDSYS_IRQ[15]     */
  .word     isrC_main     /*72:  INT 56	MDSYS_IRQ[16]     */
  .word     isrC_main     /*73:  INT 57	MDSYS_IRQ[17]     */
  .word     isrC_main     /*74:  INT 58	MDSYS_IRQ[18]     */
  .word     isrC_main     /*75:  INT 59	MDSYS_IRQ[19]     */
  .word     isrC_main     /*76:  INT 60	MDSYS_IRQ[20]     */
  .word     isrC_main     /*77:  INT 61	MDSYS_IRQ[21]     */
  .word     isrC_main     /*78:  INT 62	MDSYS_IRQ[22]     */
  .word     isrC_main     /*79:  INT 63	MDSYS_IRQ[23]     */
  .word     isrC_main     /*80:  INT 64	MDSYS_IRQ[24]     */
  .word     isrC_main     /*81:  INT 65	MDSYS_IRQ[25]     */
  .word     isrC_main     /*82:  INT 66	MDSYS_IRQ[26]     */
  .word     isrC_main     /*83:  INT 67	MDSYS_IRQ[27]     */
  .word     isrC_main     /*84:  INT 68	MDSYS_IRQ[28]     */
  .word     isrC_main     /*85:  INT 69	MDSYS_IRQ[29]     */
  .word     isrC_main     /*86:  INT 70	MDSYS_IRQ[30]     */
  .word     isrC_main     /*87:  INT 71	CM4 CSCI_MD_READ_IRQ   */
  .word     isrC_main     /*88:  INT 72	CM4 CSCI_MD_WRITE_IRQ  */
  .word     isrC_main     /*89:  INT 73	CM4 CSCI_AP_READ_IRQ   */
  .word     isrC_main     /*90:  INT 74	CM4 CSCI_AP_WRITE_IRQ  */
  .word     isrC_main     /*91:  INT 75	CM4 reserved           */
  .word     isrC_main     /*92:  INT 76	CM4 reserved           */

         

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

  .weak      NMI_Handler
  .thumb_set NMI_Handler,Default_Handler
  
  .weak      HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler
  
  .weak      MemManage_Handler
  .thumb_set MemManage_Handler,Default_Handler
  
  .weak      BusFault_Handler
  .thumb_set BusFault_Handler,Default_Handler
  
  .weak      UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_Handler
  
  .weak      SVC_Handler
  .thumb_set SVC_Handler,Default_Handler
  
  .weak      DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_Handler
  
  .weak      PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler
  
  .weak      SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler
  
 

/************************ (C) COPYRIGHT MEDIATEK *****END OF FILE****/

