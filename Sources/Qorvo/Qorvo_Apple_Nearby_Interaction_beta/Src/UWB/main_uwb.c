/**
 * @file    main_uwb.c
 * 
 * @brief   Implementation of the Accessory UWB TWR
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) DecaWave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

/* Includes */
#include "nrf_log.h"
#include "nrf_sdh.h"
#include "nrfx_wdt.h"

#include "niq.h"
#include "app.h"
#include "app_config.h"
#include "common_fira.h"
#include "ble_conn_params.h"

#include "task_signal.h"

#include "cmsis_os.h"

#include "util.h"
#include "int_priority.h"

#include "HAL_DW3000.h"

#include "fira_app.h"
#include "main_uwb.h"

uint32_t ranging_role;
static task_signal_t defaultTask;

typedef enum {
  SIGNAL_STOP_ALL   = 1<<0,
  SIGNAL_START_INIT = 1<<1,
  SIGNAL_START_RESP = 1<<2
} default_task_signal_e;

/* Static fn() prototypes */
static void 
construct_fira_param_from_config(fira_param_t *fira_param,
                                 fira_device_configure_t *config, int controller);

/**
 * @brief show current threads and stack depth
 *
 * */
static void display_threads_heap_usage(void)
{
#if configUSE_TRACE_FACILITY == 1
    char *pcWriteBuffer = malloc(1024);

    if(pcWriteBuffer)
    {
        TaskStatus_t *pxTaskStatusArray;
        volatile UBaseType_t uxArraySize, x;
        unsigned long ulTotalRunTime, ulStatsAsPercentage;

       /* Make sure the write buffer does not contain a string. */
       *pcWriteBuffer = 0x00;

       /* Take a snapshot of the number of tasks in case it changes while this
       function is executing. */
       uxArraySize = uxTaskGetNumberOfTasks();

       /* Allocate a TaskStatus_t structure for each task.  An array could be
       allocated statically at compile time. */
       pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) );

       int sz = 0;
       if( pxTaskStatusArray != NULL )
       {
          /* Generate raw status information about each task. */
          uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, (unsigned int * const)&ulTotalRunTime );

          /* For each populated position in the pxTaskStatusArray array,
          format the raw data as human readable ASCII data. */
          sz += sprintf( &pcWriteBuffer[sz], "\r\n%-16s\t%s\r\n","THREAD NAME","Stack usage");
          for( x = 0; x < uxArraySize; x++ )
          {
              uint32_t *p = pxTaskStatusArray[ x ].pxStackBase;
              uint32_t total = (p[-1] & 0xFFFFUL) - 8;
              sz += sprintf( &pcWriteBuffer[sz], "%-16s\t%d/%d\r\n",
                                 pxTaskStatusArray[ x ].pcTaskName,
                                 total - pxTaskStatusArray[ x ].usStackHighWaterMark*4,
                                 total);
          }
          /* The array is no longer needed, free the memory it consumes. */
          vPortFree( pxTaskStatusArray );

          sz += sprintf( &pcWriteBuffer[sz], "%-16s\t%d\r\n", "Free HEAP", xPortGetFreeHeapSize());
          sz += sprintf( &pcWriteBuffer[sz], "%-16s\t%d\r\n", "Min free HEAP", xPortGetMinimumEverFreeHeapSize());
       }

        NRF_LOG_INFO("%s",pcWriteBuffer);

        free(pcWriteBuffer);
    }
#endif
}

/* 
 * This task will start an Accessory as a UWB Initiator or UWB Responder
 * 
 */
/*
static void _defaultTask(const void *arg)
{
    char str[256];
    osEvent evt;
    fira_param_t *fira_param = get_fira_config();
    param_block_t* app_config = AppConfigGet();

    int Signal = SIGNAL_START_INIT | SIGNAL_START_RESP | SIGNAL_STOP_ALL;

    while(1)
    {
        osMutexRelease(defaultTask.MutexId);
        evt = osSignalWait(Signal, 10000);
        osMutexWait(defaultTask.MutexId, 0);

        nrfx_wdt_feed();

        if(evt.status == osEventTimeout)
        {
            display_threads_heap_usage();
            continue;
        }

        fira_terminate();

        disable_dw3000_irq();


        switch (evt.value.signals)
        {
        case SIGNAL_START_INIT:
          construct_fira_param_from_config(fira_param, (void*)&AppConfigGet()->s.fira_config, 1);
          fira_helper_controller((void const *)fira_param);
          break;
        case SIGNAL_START_RESP:
          construct_fira_param_from_config(fira_param, (void*)&AppConfigGet()->s.fira_config, 0);
          fira_helper_controlee((void const *)fira_param);
          break;

        case SIGNAL_STOP_ALL:
        //through
        default:

        // niq_reinit() must be called every time Session is Stopped. 
         // If not, then next session would not start 
         
          niq_reinit(); 
          break;
        }
    }
}
*/
static void _defaultTask(const void *arg)
{
    char str[256];
    osEvent evt;
    fira_param_t *fira_param = get_fira_config();
    param_block_t* app_config = AppConfigGet();

    char result[256];
    snprintf(result, sizeof(result), "_defaultTask ->\n");            
    port_tx_msg(result, strlen(result));

    int Signal = SIGNAL_START_INIT | SIGNAL_START_RESP | SIGNAL_STOP_ALL;

    while(1)
    {

        osMutexRelease(defaultTask.MutexId);
        evt = osSignalWait(Signal, 10000);
        osMutexWait(defaultTask.MutexId, 0);

        nrfx_wdt_feed();

        /*
        if(evt.status == osEventTimeout)
        {
            display_threads_heap_usage();
            snprintf(result, sizeof(result), "osEventTimeout ->\n");            
            port_tx_msg(result, strlen(result));

            continue;
        }*/

        fira_terminate();

        disable_dw3000_irq();

   

        snprintf(result, sizeof(result), "ResumeUWBTasks: evt.value_signals %d\n", evt.value.signals);            
        port_tx_msg(result, strlen(result));

               
        snprintf(result, sizeof(result), "_defaultTask - AppGetFiraRole: %d\n", AppGetFiRaRole());            
        port_tx_msg(result, strlen(result));

        
        if (ranging_role == 1) {
          construct_fira_param_from_config(fira_param, (void*)&AppConfigGet()->s.fira_config, 1);
          fira_helper_controller((void const *)fira_param);

        } else {
          construct_fira_param_from_config(fira_param, (void*)&AppConfigGet()->s.fira_config, 0);
          fira_helper_controller((void const *)fira_param);
          }
        
    }

    snprintf(result, sizeof(result), "_defaultTask <-\n");            
    port_tx_msg(result, strlen(result));
}


/*
 * @brief Starting default task on the lowest priority
 *        It will start/stop relevant UWB application
 */
int main_uwb(uint32_t ranging_role_main)
{
    ranging_role = ranging_role_main;
    char result[256];
    snprintf(result, sizeof(result), "MAIN UWB ->\n"); 
    port_tx_msg(result, strlen(result));
     
    AppConfigInit();                 /**< load the RAM Configuration parameters from NVM block */

    BoardInit();

    if(UwbInit() != DWT_SUCCESS)
    {
        APP_ERROR_HANDLER(NRF_ERROR_RESOURCES);
    }
    
    osMutexDef(defMutex);
    defaultTask.MutexId    = osMutexCreate(osMutex(defMutex));

    osThreadDef(DefTask, _defaultTask, PRIO_StartDefaultTask, 0, 1024);
    defaultTask.Handle = osThreadCreate(osThread(DefTask), NULL);

    if( !defaultTask.Handle)
    {
      APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
}


/*
 * @brief signaling to start a correct ranging device 
 */
void ResumeUwbTasks(void)
{

    osSignalSet(defaultTask.Handle, (AppGetFiRaRole() & 0x01)?(SIGNAL_START_INIT):(SIGNAL_START_RESP));
}

void StopUwbTask(void)
{
    osSignalSet(defaultTask.Handle, SIGNAL_STOP_ALL);
}


static void 
construct_fira_param_from_config(fira_param_t *fira_param, fira_device_configure_t *config, int controller)
{

    char result[256];
    snprintf(result, sizeof(result), "construct_fira_param_from_config ->\n");            
    port_tx_msg(result, strlen(result));

    /*PHY config*/
    fira_param->dwt_config. txPreambLength = DWT_PLEN_64;
    fira_param->dwt_config. rxPAC     = DWT_PAC8;
    fira_param->dwt_config. dataRate  = DWT_BR_6M8;
    fira_param->dwt_config. phrMode   = DWT_PHRMODE_STD;
    fira_param->dwt_config. phrRate   = DWT_PHRRATE_STD;
    fira_param->dwt_config. sfdTO     = 65;             
    fira_param->dwt_config. stsMode   = DWT_STS_MODE_OFF;
    fira_param->dwt_config. stsLength = DWT_STS_LEN_64;
    fira_param->dwt_config. pdoaMode  = DWT_PDOA_M0;

    /* Configure specific fira app parameters */
    fira_param->dwt_config. sfdType   = (config->SP0_PHY_Set == BPRF_SET_2)?(DWT_SFD_IEEE_4Z):(DWT_SFD_IEEE_4A);
    fira_param->dwt_config. chan      = config->Channel_Number;
    fira_param->dwt_config. txCode    = config->Preamble_Code;
    fira_param->dwt_config. rxCode    = config->Preamble_Code;


    /* MAC configure */
    // fira_param->short_addr = AR2U16(config->SRC_ADDR); /* see below */
    fira_param->session_id = config->Session_ID;
    fira_param->session.initiation_time_ms = config->UWB_Init_Time_ms;

    snprintf(result, sizeof(result), "construct - 0\n");            
    port_tx_msg(result, strlen(result));

    snprintf(result, sizeof(result), "controller: %d\n", controller);            
    port_tx_msg(result, strlen(result));

    AppSetFiRaRole(controller);

    snprintf(result, sizeof(result), "AppGetFiRaRole: %d\n", AppGetFiRaRole());            
    port_tx_msg(result, strlen(result));

    if (controller == 1) { /* Initiator: 1 */
            fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLLER;
            fira_param->session.device_role = FIRA_DEVICE_ROLE_INITIATOR;
            fira_param->short_addr = 98; /* override settings in <default_config.h> */
            fira_param->session.destination_short_address = 9;

    } else { /* Responder: 0 */
            fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLEE;
            fira_param->session.device_role = FIRA_DEVICE_ROLE_RESPONDER;
            fira_param->short_addr = 9;
            fira_param->session.destination_short_address = 98;
    }

    snprintf(result, sizeof(result), "construct - 1\n");            
    port_tx_msg(result, strlen(result));
    
    /* Only supporting Deferred DS-TWR */
    fira_param->session.ranging_round_usage = config->Ranging_Round_Usage;
    fira_param->session.rframe_config = config->Rframe_Config;
    fira_param->session.sfd_id = (config->SP0_PHY_Set == BPRF_SET_2)?(FIRA_SFD_ID_2):(FIRA_SFD_ID_0);
    fira_param->session.slot_duration_rstu = config->Slot_Duration_RSTU;
    fira_param->session.block_duration_ms = config->Block_Duration_ms;
    fira_param->session.round_duration_slots = config->Round_Duration_RSTU / config->Slot_Duration_RSTU;
    fira_param->session.multi_node_mode = config->Multi_Node_Mode;

    fira_param->session.round_hopping = config->Round_Hopping;
    fira_param->session.report_tof = config->ToF_Report;


    snprintf(result, sizeof(result), "----- short_addr: 0x%04x\n", fira_param->short_addr);            
    port_tx_msg(result, strlen(result));
    snprintf(result, sizeof(result), "----- session_id: %d\n", fira_param->session_id);            
    port_tx_msg(result, strlen(result));
    
    // fira_param->session.destination_short_address = AR2U16(config->DST_ADDR); /* see above */
    snprintf(result, sizeof(result), "----- destination_short_addr: 0x%04x\n", fira_param->session.destination_short_address);            
    port_tx_msg(result, strlen(result));

    fira_param->controlees_params.n_controlees = config->Number_of_Controlee;
    fira_param->controlees_params.controlees[0].address = fira_param->session.destination_short_address;

    fira_param->session.vupper64[7] = config->Vendor_ID[0];
    fira_param->session.vupper64[6] = config->Vendor_ID[1];
    fira_param->session.vupper64[5] = config->Static_STS_IV[0];
    fira_param->session.vupper64[4] = config->Static_STS_IV[1];
    fira_param->session.vupper64[3] = config->Static_STS_IV[2];
    fira_param->session.vupper64[2] = config->Static_STS_IV[3];
    fira_param->session.vupper64[1] = config->Static_STS_IV[4];
    fira_param->session.vupper64[0] = config->Static_STS_IV[5];

    /* Get parameters from global configuration. */
    fira_param->session.channel_number = config->Channel_Number;
    fira_param->session.preamble_code_index = config->Preamble_Code;

    /* Do not change below parameters */
    fira_param->session.rx_antenna_selection = 0;
    fira_param->session.rx_antenna_pair_azimuth = 0xFF;
    fira_param->session.rx_antenna_pair_elevation = 0xFF;
    fira_param->session.tx_antenna_selection = 1;
    fira_param->session.rx_antenna_switch = 0;
    fira_param->session.aoa_result_req = 0;
    fira_param->session.report_aoa_azimuth = 0;
    fira_param->session.report_aoa_fom = 0;

    snprintf(result, sizeof(result), "construct_fira_param_from_config <-\n");            
    port_tx_msg(result, strlen(result));
}
