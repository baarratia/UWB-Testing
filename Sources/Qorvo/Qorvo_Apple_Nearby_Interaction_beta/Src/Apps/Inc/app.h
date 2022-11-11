/**
 * @file       app.h
 *
 * @brief      Header file for app
 *
 * @author     Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */
#ifndef APP_H
#define APP_H
#include <stdbool.h>
#include "error.h"

 /* events to start/stop tasks : event group */
 enum EventTask {
     Ev_Initiator_F_Task       = 0x1,
     Ev_Responder_F_Task      = 0x2,
     //User-available max is 0x800000
     Ev_Stop_All         = 0x800000,
 };

/* System mode of operation. used to
 *
 * 1. indicate in which mode of operation system is running
 * 2. configure the access rights to command handler in control mode
 * */
typedef enum {
    mANY = 0,   /**< Used only for Commands: indicates the command can be executed in any modes below */
    mIDLE,      /**< IDLE mode */
    mInitiator, /**< FiRa Controller+Initiator mode*/
    mResponder, /**< FiRa Controlee+Responder mode*/
    mMASK      = 0xFFFF
}mode_e;

struct app_definition_s
{
    enum EventTask task_type;
    mode_e app_mode;
    void (*helper)(void const *argument);
    void (*terminate)(void);
};

typedef struct app_definition_s app_definition_t;
extern app_definition_t *known_apps;

/* Public Methods */
void AppConfigInit(void);

error_e AppGetLastError(void);
void AppSetLastError(error_e error);

mode_e AppGetMode(void);
void AppSetMode(mode_e mode);

int  AppGetFiRaRole(void);
void AppSetFiRaRole(int role);

//bool AppSwitchOffDWChipBeforeKill(void);
//int  AppGetDefaultEvent(void);
//void DefaultTaskInit(void);
//
//bool AppIsUartEnabled(void);
//void app_apptimer_stop(void);

#endif