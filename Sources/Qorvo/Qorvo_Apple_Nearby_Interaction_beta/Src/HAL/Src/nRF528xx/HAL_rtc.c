/**
 * @file    HAL_rtc.c
 * 
 * @brief   Hardware abstaction layer for RTC
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include <stdint.h>

#include "HAL_rtc.h"
#include "nrf_drv_rtc.h"
#include "int_priority.h"

/* WKUP timer counts Super Frame period.
 * The WKUP timer resolution is (30517.5) counts in 1 ns.
 */
#define WKUP_RESOLUTION_NS          (1e9f/32768.0f)

/* WKUP timer used at Ranging phase.
 * It is counts the Super Frame period, received in the Ranging Config message
 * in Tag's local time domain.
 * */
#define WKUP_RESOLUTION_US           (WKUP_RESOLUTION_NS/1e3f)

/* RTC WKUP timer counts Super Frame period.
 * The RTC WKUP timer prescaler is configured as each Tick count is 30.517 us.
 */
#define RTC_WKUP_PRESCALER          (0)

/* RTC WKUP timer counts Super Frame period.
 * The RTC WKUP timer is 24 bit counter. Counter oveflows at 2^24 - 16777216
 */
#define RTC_WKUP_CNT_OVFLW          (16777216)
#define RTC_WKUP_CNT_OVFLW_MASK     (RTC_WKUP_CNT_OVFLW-1)

static void (*rtc_callback)(void);
static uint32_t sRTC_SF_PERIOD = 3276;
static const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0); /**< Declaring an instance of nrf_drv_rtc for RTC0. */


static void rtc_disable_irq(void)
{
    NVIC_DisableIRQ(RTC0_IRQn);
}

static void rtc_enable_irq(void)
{
    NVIC_EnableIRQ(RTC0_IRQn);
}

static void rtc_set_priority_irq(void)
{
    /* configure the RTC Wakeup timer with a high priority;
     * this timer is saving global Super Frame Timestamp,
     * so we want this timestamp as stable as we can.
     *
     * */
     NVIC_SetPriority(RTC0_IRQn, PRIO_RTC_WKUP_IRQn);
}
 
static uint32_t rtc_get_counter(void)
{
    return nrf_drv_rtc_counter_get(&rtc);
}

static uint32_t rtc_get_time_elapsed(uint32_t start, uint32_t stop)
{
    //RTC is counting up, so we do stop - start to get elapsed time
    int32_t tmp = stop - start;

    // check roll over
    if (tmp < 0)
    {
        tmp += RTC_WKUP_CNT_OVFLW;    // RTC Timer overflow - 24 bit counter
    }
    return tmp;
}

//-----------------------------------------------------------------------------
/*
 * @brief   reload RTC registers after interrupt
 *          to prepare next compare match event
 * */
 static void rtc_reload(uint32_t base_cnt)
{
    uint32_t new_cc  = base_cnt + sRTC_SF_PERIOD;
    new_cc &= RTC_WKUP_CNT_OVFLW_MASK;
    nrfx_rtc_cc_set( &rtc, 0, new_cc, true);
}

//-----------------------------------------------------------------------------
/*
 * @brief   RTC callback called by the RTC interrupt
 * */
static void rtc_handler(nrf_drv_rtc_int_type_t int_type)
{

    if (int_type == NRF_DRV_RTC_INT_COMPARE0)
    {
        if(rtc_callback)
        {
            rtc_callback();
        }
    }
}

static void rtc_set_callback(void(*cb)(void))
{
    rtc_callback = cb;
}

static float rtc_get_wakup_resolution_ns(void)
{
    return WKUP_RESOLUTION_NS;
}

//-----------------------------------------------------------------------------
/*
 * @brief   setup RTC Wakeup timer
 *          period_ms is awaiting time in ms
 * */
void rtc_configure_wakeup_ms(uint32_t period_ms)
{
    ret_code_t           err_code;
    uint32_t             old_cc, new_cc;
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;

    config.prescaler = RTC_WKUP_PRESCALER;   // WKUP_RESOLUTION_US counter period

    sRTC_SF_PERIOD = (period_ms * 1e6) / WKUP_RESOLUTION_NS;

    // the timer was intialized in the rtc_init()
    nrf_drv_rtc_disable(&rtc);

    // rtc_handler will call rtcWakeUpTimerEventCallback_node()
    nrf_drv_rtc_uninit(&rtc);
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Disable tick interrupt
    nrf_drv_rtc_tick_disable(&rtc);

    //Enable Counter Compare interrupt
    old_cc = nrf_drv_rtc_counter_get(&rtc);
    new_cc = old_cc + sRTC_SF_PERIOD;
    new_cc&= RTC_WKUP_CNT_OVFLW_MASK;
    nrfx_rtc_cc_set( &rtc, 0, new_cc, true);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

//-----------------------------------------------------------------------------
/*
 * @brief   setup RTC Wakeup timer
 *          period_ns is awaiting time in ns
 * */
static void rtc_configure_wakeup_ns(uint32_t period_ns)
{
    ret_code_t           err_code;
    uint32_t             old_cc, new_cc;
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;

    config.prescaler = RTC_WKUP_PRESCALER;

    sRTC_SF_PERIOD = period_ns / WKUP_RESOLUTION_NS;

    // the timer was intialized in the rtc_init()
    nrf_drv_rtc_disable(&rtc);

    // rtc_handler will call rtcWakeUpTimerEventCallback_node()
    nrf_drv_rtc_uninit(&rtc);
    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Disable tick interrupt
    nrf_drv_rtc_tick_disable(&rtc);

    //Enable Counter Compare interrupt
    old_cc = nrf_drv_rtc_counter_get(&rtc);
    new_cc = old_cc + sRTC_SF_PERIOD;
    new_cc&= RTC_WKUP_CNT_OVFLW_MASK;
    nrfx_rtc_cc_set( &rtc, 0, new_cc, true);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

/** @brief Initialization of the RTC driver instance
 */
static int rtc_init(void)
{
    int err_code;
    nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;

    config.prescaler = RTC_WKUP_PRESCALER;   // WKUP_RESOLUTION_US counter period

    rtc_callback = NULL;

    err_code = nrf_drv_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Power on RTC instance
    nrfx_rtc_enable(&rtc);

    return err_code;
}

static void rtc_deinit(void)
{
    //stop the RTC timer
    NVIC_DisableIRQ(RTC0_IRQn);
    rtc_callback = NULL;
}

/*********************************************************************************/
/** @brief HAL RTC API structure
 */
const struct hal_rtc_s Rtc = {
    .init = &rtc_init,
    .deinit = &rtc_deinit,
    .enableIRQ = &rtc_enable_irq,
    .disableIRQ = &rtc_disable_irq,
    .setPriorityIRQ = &rtc_set_priority_irq,
    .getTimestamp = &rtc_get_counter,
    .getTimeElapsed = &rtc_get_time_elapsed,
    .reload = &rtc_reload,
    .configureWakeup_ms = &rtc_configure_wakeup_ms,
    .configureWakeup_ns = &rtc_configure_wakeup_ns,
    .getWakeupResolution_ns = &rtc_get_wakup_resolution_ns,
    .setCallback = &rtc_set_callback
};
