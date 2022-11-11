/**
 * @file    HAL_DW3000.h
 *
 * @brief   Header and definitions DW3000-related
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */


#ifndef HAL_DW3000_H
#define HAL_DW3000_H

#include <stdint.h>
#include "error.h"
#include "deca_device_api.h"

typedef enum
{
    DW_CANNOT_SLEEP,
    DW_CAN_SLEEP,
    DW_CAN_SLEEP_APP       //DW can be put to sleep by the app, after events are processed (not immediately after IRQ).
}DwCanSleepInIRQ;

typedef enum
{
    DW_NOT_SLEEPING,
    DW_IS_SLEEPING_RX,
    DW_IS_SLEEPING_IRQ,
    DW_IS_SLEEPING_FIRST
}DwEnterSleep;

typedef enum
{
    DW_SPI_READY,
    DW_SPI_SLEEPING
}DwSpiReady;

void init_dw3000_irq(void);
void enable_dw3000_irq(void);
void disable_dw3000_irq(void);
void reset_DW3000(void);
error_e port_wakeup_dw3000_fast(void);
void wakeup_device_with_io(void);
error_e port_wakeup_dw3000(void);

void port_stop_all_UWB(void);
void port_disable_wake_init_dw(void);
void port_reinit_dw_chips(void);
void port_init_dw_chip(int spiIdx);
error_e port_disable_dw_irq_and_reset(int reset);
void port_set_CanSleepInIRQ(DwCanSleepInIRQ val);
DwCanSleepInIRQ port_get_CanSleepInIRQ(void);
void port_set_DwEnterSleep(DwEnterSleep val);
DwEnterSleep port_get_DwEnterSleep(void);
void port_set_DwSpiReady(DwSpiReady val);
DwSpiReady port_get_DwSpiReady(void);
void dw_irq_init(void);

static inline int dw3000_is_AoA(void)
{
    //TODO: we should not check the ID but rather request for a property PDoA or not PDoA
    //this will avoid updating this everytime we have a new ID
    //this is here temprarilly but should be added to the driver API
    uint32_t dev_id = dwt_readdevid();
    switch(dev_id)
    {
    case DWT_DW3000_PDOA_DEV_ID:
    case DWT_DW3700_PDOA_DEV_ID:
    case DWT_DW3720_PDOA_DEV_ID:
        return AOA;  
        break;
    case DWT_DW3000_DEV_ID:
        return NON_AOA;
        break;
    default:
        return DWT_ERROR;
    }
}

extern const struct dwt_probe_s dw3000_probe_interf;

#endif /* HAL_DW3000_H */
