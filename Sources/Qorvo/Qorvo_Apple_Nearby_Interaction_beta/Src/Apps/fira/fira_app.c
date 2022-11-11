/**
 *  @file     fira_app.c
 *
 *  @brief    Fira processes control
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */
#include "dw3000.h"
#include "error.h"
#include "deca_interface.h"
#include "uwbmac.h"
#include "fira_helper.h"
#include "common_fira.h"
#include "dw3000_mcps_mcu.h"
#include "HAL_error.h"

#include "app_config.h"
#include "default_config.h"
#include "task_signal.h"
#include "int_priority.h"

static bool started = false;
static uint32_t session_id = 0;
static const uint32_t port_id = 3;

static struct dwchip_s *dw = NULL;
static task_signal_t reportTask;

static struct ranging_results *results;
static fira_param_t *fira_param;

typedef enum {
  SIGNAL_REPORT_READY = 1<<0,
} report_task_signal_e;


/* @brief Callback, will be called every time Ranging Data are ready
 * 
 */
static void report_cb(void *user_data, struct sk_buff *skb)
{
        fira_helper_parse_ranging_report(skb, results);
        kfree_skb(skb);
        if (osSignalSet(reportTask.Handle, SIGNAL_REPORT_READY) != osOK) {
                error_handler(1, _ERR_Signal_Bad);
        }
}


static void driver_configure_for_fira(struct dwchip_s *dw)
{
        static rxtx_configure_t rxtx_config_fira_app;
        
        static dwt_sts_cp_key_t dummy_stsKey = {};
        static dwt_sts_cp_iv_t  dummy_stsIv = {};

        param_block_t *pConfig = AppConfigGet();

        rxtx_config_fira_app.pdwCfg = &pConfig->dwt_config;
        rxtx_config_fira_app.frameFilter = DWT_FF_ENABLE_802_15_4;
        rxtx_config_fira_app.frameFilterMode =
                (DWT_FF_BEACON_EN | DWT_FF_DATA_EN | DWT_FF_ACK_EN |
                 DWT_FF_COORD_EN);
        rxtx_config_fira_app.txAntDelay = pConfig->s.antTx_a;
        rxtx_config_fira_app.rxAntDelay = pConfig->s.antRx_a;
        rxtx_config_fira_app.txConfig = &pConfig->s.txConfig;

        static dwt_mcps_config_t dw_conf;
        dw_conf.rxtx_config = &rxtx_config_fira_app;
        dw_conf.mode = DWT_DW_INIT;
        dw_conf.do_reset = 1;
        dw_conf.led_mode = DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK;
        dw_conf.lnapamode = DWT_TXRX_EN;
        dw_conf.bitmask_lo = DWT_INT_SPIRDY_BIT_MASK | DWT_INT_TXFRS_BIT_MASK |
                             DWT_INT_RXFCG_BIT_MASK | DWT_INT_ARFE_BIT_MASK |
                             DWT_INT_RXFSL_BIT_MASK | DWT_INT_RXSTO_BIT_MASK |
                             DWT_INT_RXPHE_BIT_MASK | DWT_INT_RXFCE_BIT_MASK |
                             DWT_INT_RXFTO_BIT_MASK | DWT_INT_RXFR_BIT_MASK |
                             DWT_INT_RXPTO_BIT_MASK;
        dw_conf.bitmask_hi = 0;
        dw_conf.int_options = DWT_ENABLE_INT_ONLY;
        dw_conf.sleep_config.mode = DWT_CONFIG | DWT_PGFCAL;
        dw_conf.sleep_config.wake = DWT_PRES_SLEEP | DWT_WAKE_CSN | DWT_SLP_EN;
        dw_conf.xtalTrim = pConfig->s.xtalTrim;
        
        dw_conf.stsKey = &dummy_stsKey;
        dw_conf.stsIv = &dummy_stsIv;

        dw_conf.loadIv = 0;
        dw_conf.event_counter = 1;
        dw_conf.cia_enable_mask = DW_CIA_DIAG_LOG_ALL;
        dw->config = &dw_conf;
}


static error_e fira_app_process_init(bool controller)
{
        session_id = fira_param->session_id;

        int r = uwbmac_init();
        assert(!r);

        dw = dw3000_mcps_alloc();
        //TODO: this call should configure SFD, PHR, disable STS, txcode, tx gain...
        //Should set everything despite pan id, short addr, ieee addr & promiscuous.
        driver_configure_for_fira(dw);

        //register driver and callbacks;
        r = dw3000_mcps_register(dw);
        assert(!r);
        //unset promiscuous to accept only filtered frames.
        uwbmac_set_promiscuous_mode(false);
        //set local short address.
        uwbmac_set_short_addr(fira_param->short_addr);
        //register report cb on the port N;
        uwbmac_register_report_callback(NULL, port_id, report_cb);
        // Set fira scheduler;
        r = fira_helper_set_scheduler();
        assert(!r);
        // init session;
        r = fira_helper_init_session(session_id);
        assert(!r);
        // Set session parameters;
        r = fira_helper_set_session_parameters(session_id,
                                               &fira_param->session);
        assert(!r);

        if (controller) {
                // Add controlee session parameters;
                r = fira_helper_add_controlees(session_id,
                                               &fira_param->controlees_params);
                assert(!r);
        }
        return _NO_ERR;
}

static void fira_app_process_start(void)
{
        int r = uwbmac_start();
        assert(!r);
        // Start FiRa session;
        r = fira_helper_start_session(session_id, port_id);
        assert(!r);
        started = true;
}

static error_e fira_app_process_terminate(void)
{
        if (started)
        {
            // Stop session;
            int r = fira_helper_stop_session(session_id);
            assert(!r);
            //Stop.
            uwbmac_stop();
            // Uninit session;
            r = fira_helper_deinit_session(session_id);
            assert(!r);
            //unregister driver;
            dw3000_mcps_unregister(dw);
            dw3000_mcps_free(dw);
            started = false;
        }
}

/* @brief Displaying of the results
 *
 * */
static void _reportTask(void const *arg)
{
        error_e ret;
        uint16_t head, tail;
        float r_m;
        char result[256];

        const int Signal = SIGNAL_REPORT_READY;

        do {
                osMutexRelease(reportTask.MutexId);
                osSignalWait(Signal, osWaitForever);
                osMutexWait(reportTask.MutexId, 0);

                for (int i = 0; i < results->n_measurements; i++)
                {
                    snprintf(result, sizeof(result), 
                        "{\"TWR\": {\"R\":%d,\"a16\":\"0x%04x\",\"S\":\"%s\",\"D cm\":%d}}\r\n",
                        results->block_index,
                        results->measurements[i].short_addr,
                        (results->measurements[i].status == 0)?("OK"):("ERR"),
                        (int)results->measurements[i].distance_mm/10
                        );
                }
                port_tx_msg(result, strlen(result));
        } while (1);
}

/* @brief Setup Report task which will display the ranging report
 * 
 * */
static void fira_setup_tasks(void)
{
        osThreadDef(repTask, _reportTask, PRIO_TagRxTask, 0, 256);
        osMutexDef(repMutex);

        reportTask.MutexId = osMutexCreate(osMutex(repMutex));
        reportTask.Handle = osThreadCreate(osThread(repTask), NULL);
        reportTask.Signal = SIGNAL_REPORT_READY; //Set for backward compatibility, not used

        results = malloc(sizeof(struct ranging_results));

        if ((reportTask.Handle == NULL) || (results == NULL)) {
                error_handler(1, _ERR_Create_Task_Bad);
        }
}


/* @fn      fira_helper
 * @brief   this is a service function which starts the FiRa TWR 
 *          top-level  application.
 * */
static void fira_helper(bool controller)
{
        error_e tmp;       

        taskENTER_CRITICAL(); /**< When the app will setup RTOS tasks, then if task has a higher priority,
                                 the kernel will start it immediately, thus we need to stop the scheduler.*/

        tmp = fira_app_process_init(controller);

        if (tmp != _NO_ERR) {
                error_handler(1, tmp);
        }

        fira_setup_tasks(); /**< "RTOS-based" : setup application tasks for FiRa TWR application. */

        fira_app_process_start(); /**< IRQ is enabled and it may generate IRQ immediately after this point */

        taskEXIT_CRITICAL(); /**< all RTOS tasks can be scheduled */
}


// Public Methods 

/* @brief
 *      Kill all task and timers related to FiRa
 *      DW3000's RX and IRQ shall be switched off before task termination,
 *      that IRQ will not produce unexpected Signal
 * */
void fira_terminate(void)
{
        fira_app_process_terminate();

        TERMINATE_STD_TASK(reportTask);

        free(results);
        results = NULL;
        uwbmac_exit();;
}

void fira_helper_controller(void const *arg)
{
  fira_param = (fira_param_t *)arg;
  fira_helper(true);
}

void fira_helper_controlee(void const *arg)
{
  fira_param = (fira_param_t *)arg;
  fira_helper(false);
}

