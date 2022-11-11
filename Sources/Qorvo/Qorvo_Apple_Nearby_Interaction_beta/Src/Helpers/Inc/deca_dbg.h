/**
 * @file      deca_dbg.h
 *
 * @brief     Debug macros for debug prints
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */
#ifndef DECA_DBG_H_
#define DECA_DBG_H_ 1

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#ifndef USE_RTT_PRINT
#define USE_RTT_PRINT           1
#endif

#if (USE_RTT_PRINT == 1)
#include "SEGGER_RTT.h"
extern char print_str_buff[256];

#define DBG_MSG(...) do{  \
    sprintf(rtt_print_str_buff, __VA_ARGS__);  \
    SEGGER_RTT_WriteString(0, rtt_print_str_buff); \
}while(0)

#define port_tx_msg(buff, len) SEGGER_RTT_WriteNoLock(0, buff, len)

#else
#define DBG_MSG(...)
#define port_tx_msg(...)
#endif

void diag_printf(char *s, ...); 


#ifdef __cplusplus
}
#endif

#endif /* DECA_DBG_H_ */

