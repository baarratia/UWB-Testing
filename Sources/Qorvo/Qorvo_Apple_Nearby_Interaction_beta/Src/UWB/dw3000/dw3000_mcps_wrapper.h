/**
 * @file   dw3000_mcps_wrapper.h
 *
 * @brief  DW3000 MAC Common Part Sublayer (MCPS)
 *
 * @author Decawave Application
 *
 * Copyright 2021 (c) Decawave Ltd, Dublin, Ireland.
 *           All rights reserved.
 */

#ifndef DW3000_MCPS_WRAPPER_H
#define DW3000_MCPS_WRAPPER_H

#include "linux/types.h"
#include "linux/skbuff.h"
#include "linux/ieee802154.h"
#include "deca_interface.h"

#include "HAL_DW3000.h"

/**
 * dw3000_rx_enable() - Enable RX
 * @dw: the DW device to put in RX mode
 * @rx_delayed:  true if RX delayed must be use
 * @date_dtu: the date at which RX must be enabled if @rx_delayed is true
 * @timeout_pac: the preamble detect timeout
 *
 * This function enable RX on DW3000, delayed or not, with a configurable
 * preamble detection timeout.
 *
 * Return: 0 on success, else a negative error code.
 */
static inline int dw3000_rx_enable(struct dwchip_s *dw, int rx_delayed,
                   u32 date_dtu, u32 timeout_pac)
{
    struct dw_rx_frame_info_s rxops = { .rx_date_dtu = date_dtu,
                        .rx_timeout_pac = timeout_pac,
                        .rx_delayed = rx_delayed };
    return dw->dwt_driver->dwt_mcps_ops->rx_enable(dw, &rxops);
}

/**
 * dw3000_rx_disable() - Disable RX
 * @dw: the DW device to put back in IDLE state
 *
 * Return: 0 on success, else a negative error code.
 */
static inline int dw3000_rx_disable(struct dwchip_s *dw)
{
    return dw->dwt_driver->dwt_mcps_ops->rx_disable(dw);
}

/**
 * dw3000_tx_frame() - prepare, execute or program TX
 * @dw: the DW device
 * @skb: TX socket buffer
 * @tx_delayed: true if TX delayed must be use
 * @tx_date_dtu: the date at which TX must be executed
 * @rx_delay_dly: positive if needed to active RX after TX otherwise null
 * @rx_timeout_pac: the preamble detect timeout
 *
 * This function prepares, executes or programs TX according to a given socket
 * buffer pointer provided by the MCPS.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_tx_frame(struct dwchip_s *dw, struct sk_buff *skb,
                  int tx_delayed, u32 tx_date_dtu,
                  int rx_delay_dly, u32 rx_timeout_pac)
{
    struct dw_tx_frame_info_s txops = { .tx_date_dtu = tx_date_dtu,
                        .rx_delay_dly = rx_delay_dly,
                        .rx_timeout_pac = rx_timeout_pac };

    txops.flag = tx_delayed ? DWT_START_TX_DELAYED : DWT_START_TX_IMMEDIATE;

    if (rx_delay_dly > 0)
        txops.flag |= DWT_RESPONSE_EXPECTED;
    if (skb == NULL)
        return dw->dwt_driver->dwt_mcps_ops->tx_frame(dw, NULL, 0,
                                  &txops);
    else
        return dw->dwt_driver->dwt_mcps_ops->tx_frame(
            dw, skb->data, skb->len + IEEE802154_FCS_LEN, &txops);
}

/**
 * dw3000_setpdoa() - set device's PDOA mode
 * @dw: the DW device
 * @mode: the PDOA mode
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_setpdoa(struct dwchip_s *dw, u8 mode)
{
    return 0;
}

/**
* dw3000_readpdoa - Read the PDOA result.
* @dw: The DW device.
*
* This is used to read the PDOA result, it is the phase difference between
* either the Ipatov and STS POA, or the two STS POAs, depending on the PDOA
* mode of operation. (POA - Phase Of Arrival).
* NOTE: To convert to degrees: float pdoa_deg =
* ((float)pdoa / (1 << 11)) * 180 / M_PI.
*
* Return: The PDOA result (signed in [1:-11] radian units).
*/
static inline s16 dw3000_readpdoa(struct dwchip_s *dw)
{
    int16_t tmp;
    dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_READPDOA, 0, (void *)&tmp);
    return tmp;
}

/**
 * dw3000_setsts() - set device's STS mode
 * @dw: the DW device
 * @mode: the STS mode
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_set_sts(struct dwchip_s *dw, u8 mode)
{
    uint8_t stsMode = mode;

    return dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_CONFIGURESTSMODE, 0,
                           (void *)&stsMode);
}

/**
 * dw3000_enable_autoack() - Enable the autoack for futures rx.
 * @dw: The DW device.
 * @force: Enable even if it was already in enable state.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_enable_autoack(struct dwchip_s *dw, bool force)
{
    dw->dwt_driver->dwt_mcps_ops->mcps_compat.ack_enable(dw, (int)true);
    return 0;
}

/**
 * dw3000_disable_autoack() - Disable the autoack for futures rx.
 * @dw: The DW device.
 * @force: Disable even if it was already in disable state.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_disable_autoack(struct dwchip_s *dw, bool force)
{
    dw->dwt_driver->dwt_mcps_ops->mcps_compat.ack_enable(dw, (int)false);
    return 0;
}

/**
 * dw3000_config_antenna_gpios() - Set configuration for all used GPIO
 * @dw: The DW device.
 *
 * This function configure all GPIO found in antenna table.
 * It is called before enabling the DW device in start() MCPS API.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_config_antenna_gpios(struct dwchip_s *dw)
{
    return 0;
}

/**
 * dw3000_set_tx_antenna() - Configure device to use selected antenna for TX
 * @dw: The DW device.
 * @antidx: The antenna to use for next TX
 *
 * Prepare the DW device to transmit frame using the specified antenna.
 * The required HW information (port, gpio and gpio value) must be set
 * correctly inside calibration data structure.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_set_tx_antenna(struct dwchip_s *dw, int antidx)
{
    return 0;
}

/**
 * dw3000_set_rx_antennas() - Set GPIOs to use selected antennas for RX
 * @dw: The DW device.
 * @ant_pair: The antennas pair to use
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_set_rx_antennas(struct dwchip_s *dw, int ant_pair)
{
    return 0;
}

/**
 * dw3000_enable() - Enable the device
 * @dw: the DW device
 *
 * This function masks the device's internal interruptions (fixed
 * configuration) then enables the IRQ linked to the device interruption line.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_enable(struct dwchip_s *dw)
{
    enable_dw3000_irq(); /**< enable DW3000 IRQ to start  */
    return 0;
}

/**
 * dw3000_disable() - Disable the device
 * @dw: the DW device
 *
 * This function disables the IRQ linked to the device interruption line,
 * unmasks the device's internal interruptions then clears its pending
 * interruptions.
 *
 * Return: zero on success, else a negative error code.
 */
static inline int dw3000_disable(struct dwchip_s *dw)
{
    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

    /* Disable further interrupt generation */
    ops->mcps_compat.set_interrupt(dw, 0xFFFFFFFFUL, 0xFFFFFFFFUL,
                       DWT_DISABLE_INT);

    /* Disable receiver and transmitter */
    return ops->ioctl(dw, DWT_FORCETRXOFF, 0, 0);
}

/**
 * dw3000_set_sts_length() - set device's STS length
 * @dw: the DW device
 * @len: length of the STS in blocks of 8 symbols
 *
 * Return: zero on success, else a negative error code.
 */
static int dw3000_set_sts_length(struct dwchip_s *dw, dwt_sts_lengths_e len)
{
    const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

    dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

    int ret = 0;

    if (config->stsLength != len) {
        ret = ops->ioctl(dw, DWT_SET_STS_LEN, 0, (void *)(&len));

        if (ret < 0) {
            return -ENOENT;
        }
        config->stsLength = len;

        ret = ops->ioctl(dw, DWT_CFG_STS, 0,
                 config); 
    }

    return (ret);
}

#endif //DW3000_MCPS_WRAPPER_H
