/*
 * Copyright (c) 2020-2021 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and
 * may be covered by patents, patent applications, and are protected by
 * trade secret and/or copyright law. Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 * 
 */
#ifndef __DW3000_H
#define __DW3000_H

#include <linux/skbuff.h>
#include "mcps802154.h"

/* Time units and conversion factor */
#define DW3000_CHIP_FREQ 499200000
#define DW3000_CHIP_PER_DTU 2
#define DW3000_CHIP_PER_DLY 512
#define DW3000_DTU_FREQ (DW3000_CHIP_FREQ / DW3000_CHIP_PER_DTU)
#define DW3000_RCTU_PER_CHIP 128
#define DW3000_RCTU_PER_DTU (DW3000_RCTU_PER_CHIP * DW3000_CHIP_PER_DTU)
/* 6.9.1.5 in 4z, for HRP UWB PHY:
   416 chips = 416 ÷ (499.2 × 10^6 ) ≈ 833.33 ns */
#define DW3000_DTU_PER_RSTU (416 / DW3000_CHIP_PER_DTU)

#define DW3000_RX_ENABLE_STARTUP_DLY 16
#define DW3000_RX_ENABLE_STARTUP_RCTU                         \
    (DW3000_RX_ENABLE_STARTUP_DLY * DW3000_CHIP_PER_DLY * \
     DW3000_RCTU_PER_CHIP)
#define DW3000_RX_ENABLE_STARTUP_DTU                          \
    (DW3000_RX_ENABLE_STARTUP_DLY * DW3000_CHIP_PER_DLY / \
     DW3000_CHIP_PER_DTU)

/* Additional information on rx. */
enum dw3000_rx_flags {
    /* Set if an automatix ack is send. */
    DW3000_RX_FLAG_AACK = BIT(0),
    /* Set if no data. */
    DW3000_RX_FLAG_ND = BIT(1),
};

#define device dwchip_s

#endif /* __DW3000_H */
