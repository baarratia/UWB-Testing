/**
 * @file    custom board.h
 * 
 * @brief   pin mapping description corresponding to DWM30001_A1 module
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef BOARD_DWM3001_A1_H
#define BOARD_DWM3001_A1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions for PCA10056
#define LEDS_NUMBER    4

#define LED_1          NRF_GPIO_PIN_MAP(0,4)  //D9 on the schematics
#define LED_2          NRF_GPIO_PIN_MAP(0,5)  //D10 on the schematics
#define LED_3          NRF_GPIO_PIN_MAP(0,22) //D11 on the schematics
#define LED_4          NRF_GPIO_PIN_MAP(0,14) //D12 on the schematics
#define LED_START      LED_1
#define LED_STOP       LED_4

#define LEDS_ACTIVE_STATE 0

#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      4
#define BSP_LED_1      5
#define BSP_LED_2      22
#define BSP_LED_3      14

#define BUTTONS_NUMBER 1

#define BUTTON_1       2

#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1}

#define BSP_BUTTON_0   BUTTON_1

#define RX_PIN_NUMBER  15
#define TX_PIN_NUMBER  19
#define CTS_PIN_NUMBER (-1)
#define RTS_PIN_NUMBER (-1)
#define HWFC           false

// Arduino board mappings
#define ARDUINO_13_PIN              NRF_GPIO_PIN_MAP(0,  3)  // used as DW3000_CLK_Pin
#define ARDUINO_12_PIN              NRF_GPIO_PIN_MAP(0, 29)  // used as DW3000_MISO_Pin
#define ARDUINO_11_PIN              NRF_GPIO_PIN_MAP(0,  8)  // used as DW3000_MOSI_Pin
#define ARDUINO_10_PIN              NRF_GPIO_PIN_MAP(1,  6)  // used as DW3000_CS_Pin
#define ARDUINO_9_PIN               NRF_GPIO_PIN_MAP(1, 19)  // used as DW3000_WKUP_Pin
#define ARDUINO_8_PIN               NRF_GPIO_PIN_MAP(1,  2)  // used as DW3000_IRQ_Pin
#define ARDUINO_7_PIN               NRF_GPIO_PIN_MAP(0, 25)  // used as DW3000_RST_Pin

#define DW3000_RST_Pin      ARDUINO_7_PIN
#define DW3000_IRQ_Pin      ARDUINO_8_PIN
#define DW3000_WUP_Pin      ARDUINO_9_PIN

// SPI defs
#define DW3000_CS_Pin       ARDUINO_10_PIN
#define DW3000_CLK_Pin      ARDUINO_13_PIN  // DWM3000 shield SPIM1 sck connected to DW1000
#define DW3000_MOSI_Pin     ARDUINO_11_PIN  // DWM3000 shield SPIM1 mosi connected to DW1000
#define DW3000_MISO_Pin     ARDUINO_12_PIN  // DWM3000 shield SPIM1 miso connected to DW1000
#define DW3000_SPI_IRQ_PRIORITY APP_IRQ_PRIORITY_LOW // 

// UART symbolic constants
#define UART_0_TX_PIN       TX_PIN_NUMBER           // DWM1001 module pin 20, DEV board name RXD
#define UART_0_RX_PIN       RX_PIN_NUMBER           // DWM1001 module pin 18, DEV board name TXD
#define DW3000_RTS_PIN_NUM      UART_PIN_DISCONNECTED
#define DW3000_CTS_PIN_NUM      UART_PIN_DISCONNECTED

#define LED_ERROR BSP_LED_0
#define DW3000_MAX_SPI_FREQ NRF_SPIM_FREQ_32M

#ifdef __cplusplus
}
#endif

#endif // PCA10100_H
