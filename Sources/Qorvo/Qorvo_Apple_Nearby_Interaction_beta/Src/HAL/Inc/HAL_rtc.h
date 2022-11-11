/**
 * @file    HAL_rtc.h
 *
 * @brief   Header for HAL_rtc
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef HAL_RTC_H
#define HAL_RTC_H
#include <stdint.h>

struct hal_rtc_s
{
    int (*init)(void);
    void (*deinit)(void);
    void (*enableIRQ)(void);
    void (*disableIRQ)(void);
    void (*setPriorityIRQ)(void);
    uint32_t (*getTimestamp)(void);
    uint32_t (*getTimeElapsed)(uint32_t start, uint32_t stop);
    void (*reload)(uint32_t time);
    void (*configureWakeup_ms)(uint32_t period_ms);
    void (*configureWakeup_ns)(uint32_t period_ns);
    float(*getWakeupResolution_ns)(void);
    void (*setCallback)(void (*cb)(void));
};

extern const struct hal_rtc_s Rtc;
#endif
