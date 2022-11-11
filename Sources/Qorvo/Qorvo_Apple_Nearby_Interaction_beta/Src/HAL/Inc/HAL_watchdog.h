/**
 * @file    HAL_usb.h
 *
 * @brief   Header for HAL_usb
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef HAL_WATCHDOG_H
#define HAL_WATCHDOG_H
void wdt_init(int ms);
void wdt_refresh(void);
#endif