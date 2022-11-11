/**
 * @file    HAL_timer.h
 *
 * @brief   Header for HAL_timer
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>

uint32_t init_timer(void);
void start_timer(volatile uint32_t * p_timestamp);
bool check_timer(uint32_t timestamp, uint32_t time);

#endif