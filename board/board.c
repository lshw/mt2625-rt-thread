/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include "board.h"
#include "hal_platform.h"
#include "hal_pinmux_define.h"
#include "hal_uart.h"
#include "hal_gpio.h"
#include "rthw.h"

#include "stdint.h"

#define NON_CACHED_HEAP_SIZE (40960u)

ATTR_RWDATA_IN_NONCACHED_RAM uint8_t non_cached_heap[ NON_CACHED_HEAP_SIZE ];

struct rt_memheap g_noncached_memheap;

void mt_noncached_heap_init(void)
{
    void *begin_addr = &non_cached_heap[0];
    void *end_addr = &non_cached_heap[NON_CACHED_HEAP_SIZE - 1];

    rt_memheap_init(&g_noncached_memheap,
                    "noncached_heap",
                    begin_addr,
                    (rt_uint32_t)end_addr - (rt_uint32_t)begin_addr);
}

void *mt_noncached_malloc(rt_size_t size)
{
    void *ptr;

    /* try to allocate in system heap */
    ptr = rt_memheap_alloc(&g_noncached_memheap, size);
    return ptr;
}

void mt_noncached_free(void *ptr)
{
    rt_memheap_free(ptr);
}
