/**
 * @file    main.c
 * 
 * @brief   Implementation of the Qorvo Apple Nearby Interaction example
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2021 (c) DecaWave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"

#include "app_timer.h"
#include "FreeRTOS.h"
#include "nrf_drv_clock.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_crypto_init.h"
#include "nrf_crypto_rng.h"

#include "cmsis_os.h"
#include "app_config.h"
#include "main_uwb.h"
#include "uwbmac.h"
#include "version.h"
#include "util.h"
#include "app.h"
#include "niq.h"

#ifndef ACCESSORY_RANGING_ROLE
#define ACCESSORY_RANGING_ROLE              (0)                                     /**< Responder 0, Initiator 1 */
#endif

extern void ble_init(char * gap_name);

extern const char ApplicationName[];                                                /**< Name of Application release. */
extern const char OsName[]; 
extern const char BoardName[];                                                      /**< Name of Target. Indicated in the advertising data. */

#define DEAD_BEEF                           0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#if NRF_LOG_ENABLED
static TaskHandle_t m_logger_thread;                                /**< Definition of Logger thread. */
#endif

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are deferred.
 *          Thread flushes all log entries and suspends. It is resumed by idle task hook.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
static void logger_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    while (1)
    {
        NRF_LOG_FLUSH();

        vTaskSuspend(NULL); // Suspend myself
    }
}
#endif //NRF_LOG_ENABLED

/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */
void vApplicationIdleHook( void )
{
#if NRF_LOG_ENABLED
     vTaskResume(m_logger_thread);
#endif
}


/**@brief Function for initializing the clock.
 */
static void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize modules.
    log_init();
    clock_init();

    // Activate deep sleep mode.
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    //ble_init((char*)BoardName);

#if NRF_LOG_ENABLED
    // Logging task is only required if NRF_LOG module is in use
    if (pdPASS != xTaskCreate(logger_thread, "LOGGER", 256, NULL, 1, &m_logger_thread))
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
#endif

    // Accessory Nearby Interaction Initialization
    /*niq_init(ResumeUwbTasks, StopUwbTask, (const void *)nrf_crypto_init, 
                                          (const void *)nrf_crypto_uninit, 
                                          (const void *)nrf_crypto_rng_vector_generate);
  */
    // Accessory instructed to act as a Responder or Initiator
    //niq_set_ranging_role(ACCESSORY_RANGING_ROLE);

    // Create Default task: it responsible for WDT and starting of "applications"
    main_uwb(ACCESSORY_RANGING_ROLE);

    // Driver version is available after probing of the DW chip
    const char ver[]= FULL_VERSION;
    const char *drv_ver = dwt_version_string();
    const char *mac_ver = uwbmac_get_version();

    char str[256];
    int sz;

    sz = sprintf(str, "Application: %s\r\n", ApplicationName);
    sz += sprintf(&str[sz], "Target: %s\r\n", BoardName);
    sz += sprintf(&str[sz], "OS: %s\r\n", OsName);
    sz += sprintf(&str[sz], "Version: %s\r\n", ver);
    sz += sprintf(&str[sz], "%s\r\n", drv_ver);    
    sz += sprintf(&str[sz], "MAC: %s\r\n", mac_ver);
    sz += sprintf(&str[sz], "ACCESSORY_RANGING_ROLE: %s\r\n", (ACCESSORY_RANGING_ROLE)?"Initiator":"Responder");
    port_tx_msg(str, sz);

    // Start FreeRTOS scheduler.
    osKernelStart();

    for (;;)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}


