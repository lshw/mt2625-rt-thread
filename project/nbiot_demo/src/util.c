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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>


char * object_ftoa(double f, char * buf, int precision)
{
    char * ptr = buf;
    char * p = ptr;
    char * p1;
    char c;
    long intPart;
    char* temp_str;

    // sign stuff
    if (f < 0) {
        f = -f;
        *ptr++ = '-';
    }

    f += (double)0.005;

    intPart = (long)f;
    f -= intPart;

    if (!intPart)
        *ptr++ = '0';
    else {
        // save start pointer
        p = ptr;

        // convert (reverse order)
        while (intPart) {
            *p++ = '0' + intPart % 10;
            intPart /= 10;
        }

        // save end pos
        p1 = p;

        // reverse result
        while (p > ptr) {
            c = *--p;
            *p = *ptr;
            *ptr++ = c;
    }

        // restore end pos
        ptr = p1;
}

    // decimal part
    if (precision) {
        // place decimal point
        *ptr++ = '.';

        // convert
        while (precision--) {
            f *= (double)10.0;
            c = (char)f;
            *ptr++ = '0' + c;
            f -= c;
        }
    }

    // terminating zero
    *ptr = 0;

    temp_str = --ptr;
    while(*temp_str != '.') {
        if(*temp_str == '0') {
            *temp_str = '\0';
        } else {
            break;
        }
        temp_str--;
    }

    if((*(temp_str+1) == '\0') && (*temp_str == '.')) {
        *(temp_str+1) = '0';
    }

    return buf;
}




int convert_double_to_chars(char* des, double num, int precision)
{
    int temp = 0;
    int index = 0;
    int val_int = 0;
    int tt_temp = 0, factor = 1;
    int working_index = 0;
    int flag = 0;
    double t1 = 0.001, gap = 0.00001;

    if((double)0.00001 > num)
    {
        flag = 1;
        num = num * (-1);
    }
    t1 = num;

    temp = (int)t1;
    while(temp > 0)
    {
        tt_temp = temp % 10;
        des[index] = '0' + tt_temp;
        ++index;
        val_int += tt_temp * factor;
        temp = temp / 10;
        factor *= 10;
    }
    if(1 < index)
    {
        --index;
    }

    // reverse
    for(working_index = 0; working_index <= index / 2; ++working_index)
    {
        temp = des[working_index];
        des[working_index] = des[index - working_index];
        des[index - working_index] = temp;
    }
    
    des[++index] = '.';
    ++index;
    
    gap = t1 - val_int;
    temp = 2;
    factor = 10;
    while(temp > 0)
    {
        val_int = (int)(gap * factor);
        des[index] = '0' + val_int;
        ++index;
        gap = gap - (double)val_int*1.0/10;
        factor *= 10;
        --temp;
    }
    if(1 == flag)
    {
        for(temp = index - 1; temp >= 0; --temp)
        {
            des[temp + 1] = des[temp];
        }
        des[0] = '-';
    }

    return (index + 1);
}


void convert_chars_to_double(char *str, int *length, double *ret_val)
{
    char *temp_str = str;
    char working_char = '\0';
    int dot_flag = 1, negative_flag = 0, len = 0;
    double val = 0.000001, va_small = 0.000001, factor_size = 10.0, small_size = 1.0;
    len = 0;

    if(NULL != temp_str)
    {
        working_char = *temp_str;
        if('-' == *temp_str)
        {
            negative_flag = 1;
            ++temp_str;
            ++len;
        }
    }else{
		return;
	}
	
    while('\0' != *temp_str)
    {
        working_char = *temp_str;
        if( ('0'<= working_char) && ('9'>= working_char) )
        {
            val *= factor_size;
            val += working_char - '0';
        }
        if('.' == working_char)
        {
            dot_flag = 1;
            factor_size = 0.10;
            break;
        }
        ++temp_str;
        ++len;
    }

    if(1 == dot_flag)
    {
        ++temp_str;
        ++len;
        while('\0' != *temp_str)
        {
            working_char = *temp_str;
            if( ('0'<= working_char) && ('9'>= working_char) )
            {
                small_size *= factor_size;
                va_small += (working_char - '0') * small_size;
            }
            ++temp_str;
            ++len;
         }
         val += va_small;
    }

    if(1 == negative_flag)
    {
        val *= (double)(-1.00000);
    }

    *length = len;
    *ret_val = val;

    return;
}

