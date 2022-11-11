/**
 * @file    deca_debug.h
 *
 * @brief   Implementation of debug functionalities
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "deca_dbg.h"

#define DIAG_BUF_LEN    0x8
#define DIAG_STR_LEN    64
//note, (DIAG_BUF_LEN*(DIAG_STR_LEN+4)) shall be < HTTP_PAGE_MALLOC_SIZE-16
// <br>=4
typedef struct
{
    uint8_t buf[DIAG_BUF_LEN][DIAG_STR_LEN];
    int      head;
}gDiagPrintFStr_t;

gDiagPrintFStr_t  gDiagPrintFStr;

void diag_printf(char *s, ...)
{
    va_list args;
    va_start (args, s);
    snprintf((char*)(&gDiagPrintFStr.buf[gDiagPrintFStr.head][0]), DIAG_STR_LEN, s, args);
    port_tx_msg(&gDiagPrintFStr.buf[gDiagPrintFStr.head][0], strlen((char*)(&gDiagPrintFStr.buf[gDiagPrintFStr.head][0])));
    gDiagPrintFStr.head = (gDiagPrintFStr.head+1)&(DIAG_BUF_LEN-1);
    va_end (args);
}

