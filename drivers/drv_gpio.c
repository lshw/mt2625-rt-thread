/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-07-09     MurphyZhao        first version
 */

#include "drv_gpio.h"
#include "hal_gpio.h"
#include "hal_eint.h"
#include "rtthread.h"
#include "rtdevice.h"

#ifdef RT_USING_PIN

static void _drv_pin_write(rt_device_t dev, rt_base_t pin, rt_base_t value)
{
    hal_gpio_pin_t gpio_pin;
    hal_gpio_direction_t gpio_direction;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return;
    }
    gpio_pin = (hal_gpio_pin_t)pin;

    hal_gpio_get_direction(gpio_pin, &gpio_direction);
    if (gpio_direction) /* output */
    {
        hal_gpio_set_output(gpio_pin, (hal_gpio_data_t)value);
    }
}

static int _drv_pin_read(rt_device_t dev, rt_base_t pin)
{
    int value = PIN_LOW;
    hal_gpio_pin_t gpio_pin;
    hal_gpio_direction_t gpio_direction;
    hal_gpio_data_t gpio_data;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return value;
    }
    gpio_pin = (hal_gpio_pin_t)pin;

    hal_gpio_get_direction(gpio_pin, &gpio_direction);
    if (gpio_direction) /* output */
    {
        hal_gpio_get_output(gpio_pin, &gpio_data);
    }
    else /* input */
    {
        hal_gpio_get_input(gpio_pin, &gpio_data);
    }

    value = ((gpio_data == HAL_GPIO_DATA_LOW) ? PIN_LOW : PIN_HIGH);

    return value;
}

static void _drv_pin_mode(rt_device_t dev, rt_base_t pin, rt_base_t mode)
{
    hal_gpio_pin_t gpio_pin;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return;
    }

    gpio_pin = (hal_gpio_pin_t)pin;
    hal_gpio_init(gpio_pin);
    hal_pinmux_set_function(gpio_pin, 0); // Set the pin to operate in GPIO mode.

    if (mode == PIN_MODE_OUTPUT)
    {
        /* output setting */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
    }
    else if (mode == PIN_MODE_INPUT)
    {
        /* input setting: not pull. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
        hal_gpio_disable_pull(gpio_pin);
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        /* input setting: pull up. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
        hal_gpio_pull_up(gpio_pin);
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        /* input setting: pull down. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_INPUT);
        hal_gpio_pull_down(gpio_pin);
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        /* output setting: od. */
        hal_gpio_set_direction(gpio_pin, HAL_GPIO_DIRECTION_OUTPUT);
        hal_gpio_disable_pull(gpio_pin);
    }
}

static rt_err_t _drv_pin_attach_irq(struct rt_device *device, rt_int32_t pin,
                                     rt_uint32_t mode, void (*hdr)(void *args), void *args)
{
    hal_eint_status_t status;
    hal_eint_number_t eint_number;
    hal_eint_config_t eint_config;

    hal_gpio_pin_t gpio_pin;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return -RT_ERROR;
    }

    gpio_pin = (hal_gpio_pin_t)pin;
    eint_number = (hal_eint_number_t)pin;

    /* 7: eint func */
    if (hal_pinmux_set_function(gpio_pin, 7) != HAL_PINMUX_STATUS_OK)
    {
        return -RT_ERROR;
    }

    switch (mode)
    {
    case PIN_IRQ_MODE_RISING:
        eint_config.trigger_mode = HAL_EINT_EDGE_RISING;
        break;
    case PIN_IRQ_MODE_FALLING:
        eint_config.trigger_mode = HAL_EINT_EDGE_FALLING;
        break;
    case PIN_IRQ_MODE_RISING_FALLING:
        eint_config.trigger_mode = HAL_EINT_EDGE_FALLING_AND_RISING;
        break;
    case PIN_IRQ_MODE_HIGH_LEVEL:
        eint_config.trigger_mode = HAL_EINT_LEVEL_HIGH;
        break;
    case PIN_IRQ_MODE_LOW_LEVEL:
        eint_config.trigger_mode = HAL_EINT_LEVEL_LOW;
        break;
    default:
        return -RT_ERROR;
    }

    eint_config.debounce_time = 10;

    /* disable eint, then enable it in _drv_pin_irq_enable function */
    if (hal_eint_mask(eint_number) != HAL_EINT_STATUS_OK)
    {
        return -RT_ERROR;
    }

	if (HAL_EINT_STATUS_OK != hal_eint_init(eint_number, &eint_config))
    {
        return -RT_ERROR;
    }

    status = hal_eint_register_callback(eint_number, hdr, args);
    return (status == HAL_EINT_STATUS_OK) ? RT_EOK : (-RT_ERROR);
}

static rt_err_t _drv_pin_dettach_irq(struct rt_device *device, rt_int32_t pin)
{
    hal_eint_status_t status;
    hal_eint_number_t eint_number;
    hal_gpio_pin_t gpio_pin;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return -RT_ERROR;
    }

    gpio_pin = (hal_gpio_pin_t)pin;
    eint_number = (hal_eint_number_t)pin;

    /* disable eint interrupt */
    status = hal_eint_mask(eint_number);
    if (status != HAL_EINT_STATUS_OK)
    {
        return -RT_ERROR;
    }

    /* 0: gpio func */
    if (hal_pinmux_set_function(gpio_pin, 0) != HAL_PINMUX_STATUS_OK)
    {
        return -RT_ERROR;
    }

    /* deinit eint */
    status = hal_eint_deinit(eint_number);
    if (status != HAL_EINT_STATUS_OK)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

static rt_err_t _drv_pin_irq_enable(struct rt_device *device, rt_base_t pin,
                                     rt_uint32_t enabled)
{
    hal_eint_status_t status;
    hal_eint_number_t eint_number;

    if ((pin < HAL_GPIO_0) && (pin >= HAL_GPIO_MAX))
    {
        return -RT_ERROR;
    }

    eint_number = (hal_eint_number_t)pin;

    if (enabled == PIN_IRQ_ENABLE)
    {
        status = hal_eint_unmask(eint_number);
    }
    else
    {
        status = hal_eint_mask(eint_number);
    }

    return (status == HAL_EINT_STATUS_OK) ? RT_EOK : (-RT_ERROR);
}

const static struct rt_pin_ops _drv_pin_ops =
{
    _drv_pin_mode,
    _drv_pin_write,
    _drv_pin_read,
    _drv_pin_attach_irq,
    _drv_pin_dettach_irq,
    _drv_pin_irq_enable,
};

int rt_hw_pin_init(void)
{
    /* TODO for init gpio clock */

    /* register 'pin' device */
    return rt_device_pin_register("pin", &_drv_pin_ops, RT_NULL);
}

#endif /* RT_USING_PIN */
