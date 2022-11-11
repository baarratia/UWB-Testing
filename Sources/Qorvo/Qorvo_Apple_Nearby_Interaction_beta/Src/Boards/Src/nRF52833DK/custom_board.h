/**
 * @file    custom_board.h
 * 
 * @brief   pin mapping description for nRF52833
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */
#ifndef BOARD_nRF52833DK_H
#define BOARD_nRF52833DK_H

#ifdef __cplusplus
extern "C" {
#endif


#include "pca10100.h"

//!!!!! warning pca10100.h contains errors!!!!
#undef ARDUINO_13_PIN
#undef ARDUINO_12_PIN
#undef ARDUINO_11_PIN
#undef ARDUINO_10_PIN
#undef ARDUINO_9_PIN
#undef ARDUINO_8_PIN
#undef ARDUINO_7_PIN
#define ARDUINO_13_PIN              NRF_GPIO_PIN_MAP(0, 23)  // Digital pin 13
#define ARDUINO_12_PIN              NRF_GPIO_PIN_MAP(0, 22)  // Digital pin 12
#define ARDUINO_11_PIN              NRF_GPIO_PIN_MAP(0, 21)  // Digital pin 11
#define ARDUINO_10_PIN              NRF_GPIO_PIN_MAP(0, 20)  // Digital pin 10
#define ARDUINO_9_PIN               NRF_GPIO_PIN_MAP(0, 19)  // Digital pin 9
#define ARDUINO_8_PIN               NRF_GPIO_PIN_MAP(0, 17)  // Digital pin 8
#define ARDUINO_7_PIN               NRF_GPIO_PIN_MAP(0, 8) // Digital pin 7

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
#define DW3000_RTS_PIN_NUM  UART_PIN_DISCONNECTED
#define DW3000_CTS_PIN_NUM  UART_PIN_DISCONNECTED

#define LED_ERROR BSP_LED_0
#define DW3000_MAX_SPI_FREQ NRF_SPIM_FREQ_32M



#ifdef __cplusplus
}
#endif

#endif // PCA10100_H
