/**
 * @file    HAL_usb.h
 *
 * @brief   Header for HAL_usb
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef HAL_USB_H
#define HAL_USB_H
#include <stdbool.h>

bool deca_usb_transmit(uint8_t *tx_buffer, int size);
void deca_usb_init(void);
void InterfaceUsbUpdate(void);
#endif
