/**
 * @file   dw3000_mcps_mcu.h
 *
 * @brief  DW3000 MAC Common Part Sublayer (MCPS)
 *
 * @author Decawave Application
 *
 * Copyright 2021 (c) Decawave Ltd, Dublin, Ireland.
 *           All rights reserved.
 */

#ifndef __DW3000_MCPS_MCU_H
#define __DW3000_MCPS_MCU_H

struct ieee802154_hw *ieee802154_hw_alloc(void);
struct ieee802154_hw *ieee802154_hw_free(void);

struct dwchip_s *dw3000_mcps_alloc(void);
int dw3000_mcps_register(struct dwchip_s *dw);
void dw3000_mcps_unregister(struct dwchip_s *dw);
void dw3000_mcps_free(struct dwchip_s *dw);

#endif /* __DW3000_MCPS_MCU_H */
