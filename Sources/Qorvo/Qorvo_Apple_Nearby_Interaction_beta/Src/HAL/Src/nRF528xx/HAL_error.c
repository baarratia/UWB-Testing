/**
 * @file    HAL_error.c
 * 
 * @brief   Error handlers
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include "nrf_drv_wdt.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include "boards.h"
#include "HAL_error.h"
#include "boards.h"
#include "cmsis_os.h"

extern nrf_drv_wdt_channel_id m_channel_id;

static int lastErrorCode;

void error_handler(int block, error_e err)
{
    lastErrorCode = err;

    /* Flash Error Led*/
    while(block)
    {
        for(int i = err; i>0; i--)
        {
            for(int j = 3; j>0; j--)
            {
                nrf_drv_wdt_channel_feed(m_channel_id);    //WDG_Refresh

                nrf_gpio_pin_write(LED_ERROR, 0);
                nrf_delay_ms(100);
                nrf_gpio_pin_write(LED_ERROR, 1);
                nrf_delay_ms(100);
            }
            nrf_delay_ms(1000);
        }
    }
}

int get_lastErrorCode(void)
{
    return lastErrorCode;
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    ASSERT(false);
}
