/**
 * @file    EventManager.c
 * 
 * @brief   Event manager functionalities implmentation
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include <stdint.h>
#include <FreeRTOS.h>
#include <event_groups.h>

#include "EventManager.h"

static EventGroupHandle_t xStartTaskEvent;

void EventManagerInit(void)
{
    xStartTaskEvent = xEventGroupCreate(); /**< xStartTaskEvent indicates which tasks to be started */
}

void EventManagerSetBits(uint32_t uxBitsToSet )
{
    xEventGroupSetBits(xStartTaskEvent, (const EventBits_t)uxBitsToSet);
}

uint32_t EventManagerWaitBits(uint32_t bitsWaitForAny, uint32_t timeout)
{
    return (uint32_t) xEventGroupWaitBits(xStartTaskEvent,  (EventBits_t)bitsWaitForAny, pdTRUE, pdFALSE, (TickType_t)timeout );
}