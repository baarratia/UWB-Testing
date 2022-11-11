/**
 * @file    dw3000_xtal_trim.c
 *
 * @brief   XTAL trimming implementaion for DW3xxx
 *
 * @author Decawave Applications
 * 
 * @attention Copyright 2020-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */

#include "dw3000_xtal_trim.h"
#include "stdlib.h"

/*
 * @brief   ISR level (need to be protected if called from APP level)
 * @param   int clkOffset_pphm 
 *
 * @note    twr_info_t structure has two members xtaltrim - current trimmed value and
 *          clkOffset_pphmm, these change the DW3000 system clock and shall be applied
 *          when DW3000 is not in active Send/Receive state.
 * */
void trim_XTAL_proc(struct dwchip_s *dw, uint8_t *xtaltrim, int clkOffset_pphm)
{
    unsigned tmp = abs(clkOffset_pphm);

    if(tmp > TARGET_XTAL_OFFSET_VALUE_PPHM_MAX ||
       tmp < TARGET_XTAL_OFFSET_VALUE_PPHM_MIN)
    {
        int8_t tmp8 = (uint8_t) *(xtaltrim);
        tmp8 -= (int8_t)(((TARGET_XTAL_OFFSET_VALUE_PPHM_MAX + TARGET_XTAL_OFFSET_VALUE_PPHM_MIN)/2 + clkOffset_pphm) * AVG_TRIM_PER_PPHM);
        tmp8 = (tmp8 > XTAL_TRIM_BIT_MASK)?(XTAL_TRIM_BIT_MASK):(tmp8 < 0)?(0):(tmp8);
        *xtaltrim = (uint8_t)tmp8;

        /* Configure new Crystal Offset value */
        dw->dwt_driver->dwt_ops->ioctl(dw, DWT_SETXTALTRIM, 0, (void *)&(*xtaltrim));
    }
}