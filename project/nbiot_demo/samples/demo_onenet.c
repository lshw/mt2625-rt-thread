/*
 * This file is part of the nbiot connection to the onenet platform
 * Copyright (c) 2018-2030, longmain Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-11-01     longmain     first version
 * 2019-11-08     MurphyZhao   format
 */

#include <stdio.h>
#include "string.h"
#include <rtthread.h>

#if defined(NB_USING_ONENET) && defined(NB_ONENET_DEMO)

#include "nb_onenet.h"

#define LOG_TAG "demo.onenet"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

/**
 * define onenet stream
*/
nb_onenet_stream_t temp_object;
nb_onenet_stream_t hump_object;
nb_onenet_stream_t light0_object;
nb_onenet_stream_t inter_object;
nb_onenet_stream_t opaque_object;
nb_onenet_stream_t float_object;
nb_onenet_stream_t string_object;

void demo_onenet(void *arg)
{
	float temp_value = 75.8, hump_value = 35.6;
	rt_bool_t light_value = RT_FALSE;
	rt_uint32_t int_value = 12;
	rt_uint8_t *string_value = (rt_uint8_t *)"303132333435";
	rt_uint8_t *opaque_value = (rt_uint8_t *)"12345678";
	float float_value = 0.55;

	if (!nb_onenet_is_inited())
	{
		nb_onenet_init_environment();
	}

	/**
	 * add new object
	*/
	temp_object = nb_onenet_object_init(3303, 0, 5700, 1, "1", 1, 0, nb_onenet_value_float);
	if (temp_object == RT_NULL)
	{
		LOG_E("temp object create failure");
	}

	hump_object = nb_onenet_object_init(3304, 0, 5700, 1, "1", 1, 0, nb_onenet_value_float);
	if (hump_object == RT_NULL)
	{
		LOG_E("hump object create failure");
	}

	float_object = nb_onenet_object_init(3203, 0, 5650, 1, "1", 2, 0, nb_onenet_value_float);
	if (float_object == RT_NULL)
	{
		LOG_E("float object create failure");
	}

	string_object = nb_onenet_object_init(3203, 0, 5750, 1, "1", 2, 0, nb_onenet_value_string);
	if (string_object == RT_NULL)
	{
		LOG_E("string object create failure");
	}

	light0_object = nb_onenet_object_init(3311, 0, 5850, 1, "1", 1, 0, nb_onenet_value_bool);
	if (light0_object == RT_NULL)
	{
		LOG_E("light object create failure");
	}

	inter_object = nb_onenet_object_init(3306, 0, 5851, 1, "1", 1, 0, nb_onenet_value_integer);
	if (inter_object == RT_NULL)
	{
		LOG_E("light object create failure");
	}

	opaque_object = nb_onenet_object_init(3315, 0, 5605, 1, "1", 0, 1, nb_onenet_value_opaque);
	if (opaque_object == RT_NULL)
	{
		LOG_E("light object create failure");
	}

	if (nb_onenet_quick_start() != RT_EOK)
	{
		LOG_E("ONENET REG FAILED");
	}
	else
	{
		LOG_D("ONENET REG SUCCESS");
	}

	nb_onenet_notify(temp_object, 1, (nb_onenet_value_t)&temp_value, 0);
	nb_onenet_notify(hump_object, 1, (nb_onenet_value_t)&hump_value, 0);
	nb_onenet_notify(light0_object, 1, (nb_onenet_value_t)&light_value, 0);
	nb_onenet_notify(inter_object, 1, (nb_onenet_value_t)&int_value, 0);
	nb_onenet_notify(float_object, 1, (nb_onenet_value_t)&float_value, 0);

	LOG_D("string_len=%d; opaque_len=%d", rt_strlen((const char *)string_value), rt_strlen((const char *)opaque_value));
	nb_onenet_notify(string_object, rt_strlen((const char *)string_value), (nb_onenet_value_t)&string_value, 0);
	nb_onenet_notify(opaque_object, rt_strlen((const char *)opaque_value), (nb_onenet_value_t)&opaque_value, 0);

	rt_thread_mdelay(1000);
	if (nb_onenet_update_time(0) == RT_ERROR)
	{
		LOG_D("update time failed");
	}

	rt_thread_mdelay(1000);
	while (1)
	{
		rt_thread_mdelay(3000);
		if (nb_onenet_get_connect() == RT_EOK)
		{
			if (nb_onenet_notify_with_ack(temp_object, 1, (nb_onenet_value_t)&temp_value, 0) != RT_EOK)
			{
				LOG_D("notify error");
			}
			else
			{
				LOG_D("notify success");
			}
		}
	}
	return;
}

static void _demo_onenet(void)
{
	rt_thread_t tid;
	tid = rt_thread_create("demo_onenet", demo_onenet, NULL, 8192, 77, 10);
	if (tid)
	    rt_thread_startup(tid);
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT_ALIAS(_demo_onenet, demo_onenet, nbiot onenet test);
#endif /* FINSH_USING_MSH */

#endif /* NB_USING_ONENET && NB_ONENET_DEMO */
