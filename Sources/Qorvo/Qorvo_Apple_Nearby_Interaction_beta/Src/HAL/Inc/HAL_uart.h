/**
 * @file    HAL_uart.h
 *
 * @brief   Header for HAL_uart
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

// timeout for UART inactivity to switch it off
#include <stdbool.h>
#include "circular_buffers.h"
//#include "task_signal.h"

#define UART_OFF_TIMEOUT      30000

bool IsUartDown(void);
void SetUartDown(bool val);
//void deca_uart_init(data_circ_buf_t *_uartRx, task_signal_t *_task);
void deca_uart_close(void);
void deca_uart_init(void);
void deca_uart_receive(void);
int deca_uart_transmit(uint8_t *ptr, uint16_t sz);
int deca_uart_putc(uint8_t car);

