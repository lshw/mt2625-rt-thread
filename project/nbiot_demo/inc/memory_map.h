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

#ifndef __MEMORY_MAP_H__
#define __MEMORY_MAP_H__

#define FLASH_BASE  0x08000000

#define BL_BASE    0x08002000
#define BL_LENGTH  0x00010000    /* 64kB */

#define RTOS_BASE    0x08012000
#define RTOS_LENGTH  0x00236000   /* 2264kB */

#define FOTA_RESERVED_BASE    0x08248000
#define FOTA_RESERVED_LENGTH  0x0015D000   /* 1396kB */


#define ROM_NVDM_BASE    0x083A5000
#define ROM_NVDM_LENGTH  0x00041000   /* NVDM PART1 :260kB */

#define MINI_DUMP_BASE    0x083E6000
#define MINI_DUMP_LENGTH  0x00006000   /* NVDM PART2 :24kB */

#define EPO_RESERVED_BASE    0x083F0000
#define EPO_RESERVED_LENGTH  0x00010000   /* 64kB */

#define RAM_BASE    0x00000000
#define RAM_LENGTH  0x00200000   /* 2048kB */

#define VRAM_BASE    0x10000000
#define VRAM_LENGTH  0x00200000   /* 2048kB */

#define RAM_MD_BASE    0x00200000
#define RAM_MD_LENGTH  0x00200000   /* 2048kB */

#define VRAM_MD_BASE    0x10200000
#define VRAM_MD_LENGTH  0x00200000   /* 2048kB */

#define TCM_BASE      0x04000000
#define TCM_LENGTH    0x00004000  /* 16kB */

#define TCM_MD_BASE      0x04004000
#define TCM_MD_LENGTH    0x0000c000  /* 48kB */

#define SYSRAM_BASE      0x04200000
#define SYSRAM_LENGTH    0x00008000  /* 32kB */

#define RETSRAM_PER_BANK_LENGTH    0x00000800  /* 2kB */

#define RETSRAM_BASE      0x04300000
#define RETSRAM_LENGTH    0x00000800  /* 2kB */

#define RETSRAM_MD_BASE      0x04300800
#define RETSRAM_MD_LENGTH    0x00001800  /* 6kB */

#endif

