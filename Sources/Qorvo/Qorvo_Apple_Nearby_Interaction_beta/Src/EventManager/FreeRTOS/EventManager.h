/**
 * @file    EventManager.h
 * 
 * @brief   Event manager header file
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H
void EventManagerInit(void);
void EventManagerSetBits(uint32_t uxBitsToSet );
uint32_t EventManagerWaitBits(uint32_t bitsWaitForAny, uint32_t timeout);
#endif
