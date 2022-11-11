/**
 * @file      FreeRTOS_idle_hook.c
 *
 * @brief     Implementation of FreeRTOS idle hooks
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include "FreeRTOS.h"
#include "task.h"

#include "nrf_drv_rtc.h"
#include "nrf_sdh.h"

#if (configUSE_TICKLESS_IDLE == 2) 
/*
 * @fn      vPortSuppressTicksAndSleep
 * @brief   The function that is called by portSUPPRESS_TICKS_AND_SLEEP()
 * @param   [in] xExpectedIdleTime - time to sleep in FreeRTOS ticks
 */
void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    /*
     * Implementation note:
     *
     * To help debugging the option configUSE_TICKLESS_IDLE_SIMPLE_DEBUG was presented.
     * This option would make sure that even if program execution was stopped inside
     * this function no more than expected number of ticks would be skipped.
     *
     * Normally RTC works all the time even if firmware execution was stopped
     * and that may lead to skipping too much of ticks.
     */

    TickType_t enterTime;

    /* Make sure the SysTick reload value does not overflow the counter. */
    if ( xExpectedIdleTime > portNRF_RTC_MAXTICKS - configEXPECTED_IDLE_TIME_BEFORE_SLEEP )
    {
        xExpectedIdleTime = portNRF_RTC_MAXTICKS - configEXPECTED_IDLE_TIME_BEFORE_SLEEP;
    }
    /* Block all the interrupts globally */
    if (nrf_sdh_is_enabled())
    {
      do{
          uint8_t dummy = 0;
          uint32_t err_code = sd_nvic_critical_region_enter(&dummy);
          APP_ERROR_CHECK(err_code);
      }while (0);
    }
    else
    {
      __disable_irq();
    }

    enterTime = nrf_rtc_counter_get(portNRF_RTC_REG);

    if ( eTaskConfirmSleepModeStatus() != eAbortSleep )
    {
        TickType_t xModifiableIdleTime;
        TickType_t wakeupTime = (enterTime + xExpectedIdleTime) & portNRF_RTC_MAXTICKS;

        /* Stop tick events */
        nrf_rtc_int_disable(portNRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

        /* Configure CTC interrupt */
        nrf_rtc_cc_set(portNRF_RTC_REG, 0, wakeupTime);
        nrf_rtc_event_clear(portNRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);
        nrf_rtc_int_enable(portNRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);

        __DSB();

        /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
         * set its parameter to 0 to indicate that its implementation contains
         * its own wait for interrupt or wait for event instruction, and so wfi
         * should not be executed again.  However, the original expected idle
         * time variable must remain unmodified, so a copy is taken. */
        xModifiableIdleTime = xExpectedIdleTime;
        configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
        if ( xModifiableIdleTime > 0 )
        {
#if 1  // With FreeRTOS sd_app_evt_wait increases power consumption with FreeRTOS compared to _WFE (NRFFOSDK-11174)
#ifdef SOFTDEVICE_PRESENT
            if (nrf_sdh_is_enabled())
            {
                uint32_t err_code = sd_app_evt_wait();
                APP_ERROR_CHECK(err_code);
            }
            else
#endif
#endif // (NRFFOSDK-11174

// errata #87, 3.12 Errata_v1.6
#if (__FPU_USED == 1)
            __set_FPSCR(__get_FPSCR() & ~(0x0000009F));
            (void) __get_FPSCR();
            NVIC_ClearPendingIRQ(FPU_IRQn);
#endif // errata #87, 3.12 Errata_v1.6

            if (nrf_sdh_is_enabled())
            {
                uint32_t err_code = sd_app_evt_wait();
                APP_ERROR_CHECK(err_code);
            }
            else
            {
                /* No SD -  we would just block interrupts globally.
                * BASEPRI cannot be used for that because it would prevent WFE from wake up.
                */
                do{
                    __WFI();
                } while (0 == (NVIC->ISPR[0] | NVIC->ISPR[1]));
            }
        }

        configPOST_SLEEP_PROCESSING( xExpectedIdleTime );

        nrf_rtc_int_disable(portNRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);
        nrf_rtc_event_clear(portNRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);

        /* Correct the system ticks */
        {
            TickType_t diff;
            TickType_t exitTime;

            nrf_rtc_event_clear(portNRF_RTC_REG, NRF_RTC_EVENT_TICK);
            nrf_rtc_int_enable (portNRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

            exitTime = nrf_rtc_counter_get(portNRF_RTC_REG);
            diff =  (exitTime - enterTime) & portNRF_RTC_MAXTICKS;

            /* It is important that we clear pending here so that our corrections are latest and in sync with tick_interrupt handler */
            NVIC_ClearPendingIRQ(portNRF_RTC_IRQn);

            if ((configUSE_TICKLESS_IDLE_SIMPLE_DEBUG) && (diff > xExpectedIdleTime))
            {
                diff = xExpectedIdleTime;
            }

            if (diff > 0)
            {
                vTaskStepTick(diff);
            }
        }
    }

    if (nrf_sdh_is_enabled())
    {    
      uint32_t err_code = sd_nvic_critical_region_exit(0);
      APP_ERROR_CHECK(err_code);
    }
    else
    {
      __enable_irq();
    }

} 

#endif //configUSE_TICKLESS_IDLE == 2


