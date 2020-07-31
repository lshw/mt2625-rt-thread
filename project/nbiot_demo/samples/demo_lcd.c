/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-08-11     MurphyZhao        first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#include "lcd_st7789_spi.h"
#include "lcd_font.h"
#include "lcd_image.h"

#if defined(NB_LCD_DEMO)
#define TEST_DEV_NAME   "spi0"

#define DBG_TAG               "demo.lcd"
#define DBG_LVL               DBG_LOG
#include <rtdbg.h>

int mtk_spi_lcd_sample(void)
{
    const char *dev_name = "lcd";
    LOG_I("Test <%s> device", dev_name);

    lcd_clear(WHITE);

    /* Set the background color to white and the foreground color to black */
    lcd_set_color(WHITE, BLACK);
    lcd_show_image(0, 43, 240, 69, image_rttlogo);
    rt_thread_mdelay(1000);

    lcd_set_color(WHITE, GRAY187);
    lcd_draw_line(192 - 4, 0, 192 - 4, 240);

    lcd_set_color(WHITE, BLACK);
    lcd_show_string(192, 0, 24, "Powered by RT-Thread");

    lcd_set_color(WHITE, GRAY187);
    lcd_draw_line(192 + 28, 0, 192 + 28, 240);

    LOG_I("Test <%s> device end", dev_name);
    return RT_EOK;
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(mtk_spi_lcd_sample, demo_lcd, spi lcd test);
#endif /* FINSH_USING_MSH */
#endif /* NB_LCD_DEMO */
