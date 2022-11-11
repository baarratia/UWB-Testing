/*
 * @file       app.c
 *
 * @brief      Application configuration
 *
 * @author     Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "app.h"
#include "HAL_button.h"
#include "HAL_error.h"
#include "HAL_watchdog.h"
#include "HAL_timer.h"
#include "app_config.h"

static param_block_t *ramConfig = NULL;

void AppConfigInit(void)
{
    load_bssConfig();                 /**< load the RAM Configuration parameters from NVM block */
    ramConfig = get_pbssConfig();
}

param_block_t* AppConfigGet(void)
{
    return ramConfig;
}

error_e AppGetLastError(void)
{
    return ramConfig->s.last_error;
}

void AppSetLastError(error_e error)
{
    ramConfig->s.last_error = error;
}

mode_e AppGetMode(void)
{
    return ramConfig->s.mode;
}

void AppSetMode(mode_e _mode)
{
    ramConfig->s.mode = _mode;
}

int AppGetFiRaRole(void)
{
    return (int)(ramConfig->s.fira_config.role);
}

void AppSetFiRaRole(int role)
{
    ramConfig->s.fira_config.role = (role)?(1):(0);
}

