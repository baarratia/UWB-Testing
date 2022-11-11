/**
 * @file    custom_board.h
 * 
 * @brief   pin mapping description corresponding to nRF52840DK module
 *
 * @author  Decawave Sofware
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */
#ifndef BOARD_TYPE2AB_EVB_H
#define BOARD_TYPE2AB_EVB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

/**************************/
/*******   LEDS  **********/
/**************************/
#define BSP_LED_0      21
#define BSP_LED_1      28
#define LED_1          NRF_GPIO_PIN_MAP(0,BSP_LED_0)
#define LED_2          NRF_GPIO_PIN_MAP(0,BSP_LED_1)
#define LED_ERROR      BSP_LED_0
#define LEDS_ACTIVE_STATE 0
#define LEDS_INV_MASK  LEDS_MASK
#define LEDS_NUMBER    2
#define LEDS_LIST { LED_1, LED_2}

/*****************************/
/*******   BUTTONS  **********/
/*****************************/
#define BUTTONS_NUMBER 1
#define BUTTON_1       30
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP
#define BUTTONS_ACTIVE_STATE 0
#define BUTTONS_LIST { BUTTON_1}
#define BSP_BUTTON_0   BUTTON_1

/****************************/
/*******   DW3000  **********/
/****************************/
#define DW3000_RST_Pin      NRF_GPIO_PIN_MAP(0,15)
#define DW3000_IRQ_Pin      NRF_GPIO_PIN_MAP(0,25)
#define DW3000_WUP_Pin      NRF_GPIO_PIN_MAP(1,2)
#define DW3000_CS_Pin       NRF_GPIO_PIN_MAP(0,20)
#define DW3000_CLK_Pin      NRF_GPIO_PIN_MAP(0,16)
#define DW3000_MOSI_Pin     NRF_GPIO_PIN_MAP(0,17)
#define DW3000_MISO_Pin     NRF_GPIO_PIN_MAP(0,23)
#define DW3000_SPI_IRQ_PRIORITY APP_IRQ_PRIORITY_LOW
#define DW3000_MAX_SPI_FREQ NRF_SPIM_FREQ_32M


/**************************/
/*******   UART  **********/
/**************************/
#define RX_PIN_NUMBER  8
#define TX_PIN_NUMBER  7
#define UART_0_TX_PIN       TX_PIN_NUMBER           // DWM1001 module pin 20, DEV board name RXD
#define UART_0_RX_PIN       RX_PIN_NUMBER           // DWM1001 module pin 18, DEV board name TXD
#define RTS_PIN_NUMBER      UART_PIN_DISCONNECTED
#define CTS_PIN_NUMBER      UART_PIN_DISCONNECTED
#define HWFC           false


#ifdef __cplusplus
}
#endif

#endif // PCA10100_H
