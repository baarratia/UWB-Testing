/**
 * @file    dw3000_xtal_trim.h
 *
 * @brief   XTAL trimming implementaion for DW3xxx
 *
 * @author Decawave Applications
 * 
 * @attention Copyright 2020-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */
#ifndef __DW3000_XTAL_TRIM_H
#define __DW3000_XTAL_TRIM_H  (1)

#include <stdint.h>
#include "deca_device_api.h"
#include "deca_interface.h"

/* Minimal value of the target XTAL offset, hundreds of ppm, i.e. 1ppm = 100, 10ppm = 1000 */
#define TARGET_XTAL_OFFSET_VALUE_PPHM_MIN   (0)
/* Maximum value of the target XTAL offset, hundreds of ppm, i.e. 1ppm = 100, 10ppm = 1000 */
#define TARGET_XTAL_OFFSET_VALUE_PPHM_MAX   (100)

/* The typical trimming range of DW3000 (with 2pF external caps is ~48ppm (-30ppm to +18ppm) over all steps */
#define AVG_TRIM_PER_PPHM                   ((XTAL_TRIM_BIT_MASK+1)/48.0f/100) /* Trimming per 1 pphm*/


/*
 * @brief   ISR level (need to be protected if called from APP level)
 * @param   int clkOffset_pphm <- RX clock offset
 *          uint8_t *xtaltrim - current trimming value
 *
 * @note    twr_info_t structure has two members xtaltrim - current trimmed value and
 *          clkOffset_pphmm, these change the DW3000 system clock and shall be applied
 *          when DW3000 is not in active Send/Receive state.
 * */
void trim_XTAL_proc(struct dwchip_s *dw, uint8_t *xtaltrim, int clkOffset_pphm);


#endif /* __DW3000_XTAL_TRIM_H */
