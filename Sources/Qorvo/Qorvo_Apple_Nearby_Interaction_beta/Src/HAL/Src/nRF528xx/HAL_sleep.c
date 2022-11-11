/**
 * @file    HAL_sleep.c
 *
 * @brief   platform dependent sleep implementation
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include <stdbool.h>
#include "deca_device_api.h"
#include "HAL_timer.h"
#include "nrf_delay.h"


/* Wrapper function to be used by decadriver. Declared in deca_device_api.h */
void deca_sleep(unsigned int time_ms)
{
    nrf_delay_ms(time_ms);
}

/* @fn    usleep
 * @brief precise usleep() delay
 * */
void usleep(unsigned long time_us)
{
    nrf_delay_us(time_us);
}

/* Wrapper function to be used by decadriver. Declared in deca_device_api.h */
void deca_usleep(unsigned long time_us)
{
    usleep(time_us);
}
