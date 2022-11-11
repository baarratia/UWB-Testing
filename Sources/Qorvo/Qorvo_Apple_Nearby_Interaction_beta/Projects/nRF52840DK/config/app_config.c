/**
 * @file       config.c
 *
 * @brief      supports NVM and bss configuration sections:
 *             defaultFConfig : section in RAM, where default parameters are saved and is not re-writabele.
 *              FCONFIG_ADDR  : section in NVM, where current parameters are saved and this is re-writabele.
 *                 bssConfig  : section in RAM, which is representing config data exist in FCONFIG_ADDR.
 *
 *             application on startup shall load_bssConfig() : this will copy data from FCONFIG_ADDR -> tmpConfig
 *             Accessing to variables is by pointer get_pbssConfig();
 *
 *             if application wants to re-write data in FCONFIG_ADDR, use save_bssConfig(*newRamParametersBlock);
 *
 *             NOTE: The code is very MCU dependent and save will work with nRF52840 only
 *
 * @author     Decawave Applications
 *
 * @attention  Copyright 2017-2021 (c) DecaWave Ltd, Dublin, Ireland.
 *             All rights reserved.
 */

#include <stdint.h>
#include <string.h>

#include "deca_device_api.h"
#include "version.h"

#include "app_config.h"
#include "nrf_nvmc.h"

//------------------------------------------------------------------------------
extern const param_block_t FConfig;
extern const param_block_t defaultFConfig;

/* run-time parameters block.
 *
 * This is the RAM image of the FCONFIG_ADDR .
 *
 * Accessible from application by get_pbssConfig function after load_bssConfig()
 *
 * */
static param_block_t tmpConfig __attribute__((aligned(FCONFIG_SIZE)));

//------------------------------------------------------------------------------
// Implementation

/*
 * @brief get pointer to run-time bss param_block_t block
 *
 * */
param_block_t *get_pbssConfig(void)
{
    return &tmpConfig;
}


/* @fn      load_bssConfig
 * @brief   copy parameters from NVM to RAM structure.
 *
 *          assumes that memory model in the MCU of .text and .bss are the same
 * */
void load_bssConfig(void)
{
    memcpy(&tmpConfig, &FConfig, sizeof(tmpConfig));
}

/* @fn      restore_bssConfig
 * @brief   copy parameters from default RAM section to RAM structure.
 *
 *          assumes that memory model in the MCU of .text and .bss are the same
 * */
void restore_bssConfig(void)
{
    nrf_nvmc_page_erase((uint32_t)&FConfig);
    nrf_nvmc_write_bytes((uint32_t)&FConfig,  (const uint8_t *)&defaultFConfig, FCONFIG_SIZE);
    load_bssConfig();
}

/* @brief    save pNewRamParametersBlock to FCONFIG_ADDR
 * @return  _NO_ERR for success and error_e code otherwise
 * */
error_e save_bssConfig(const param_block_t * pNewRamParametersBlock)
{
    nrf_nvmc_page_erase((uint32_t)&FConfig);
    nrf_nvmc_write_bytes((uint32_t)&FConfig, (const uint8_t*)pNewRamParametersBlock, FCONFIG_SIZE);

    return (_NO_ERR);
}
