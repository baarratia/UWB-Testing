/**
 * @file    dw3000_mcps_mcu.c
 *
 * @brief   DW3000 MAC Common Part Sublayer (MCPS)
 *
 * @author Decawave Applications
 * 
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */

#include <linux/bits.h>
#include <linux/errno.h>

#include <stddef.h>
#include <cmsis_os.h>

#include "_types.h"

#include "mcps802154.h"

#include "HAL_DW3000.h"
#include "HAL_error.h"
#include "HAL_rtc.h"

#include "dw3000.h"
#include "translate.h"

#include "deca_interface.h"
#include "dw3000_calib_mcu.h"
#include "dw3000_mcps_wrapper.h"
#include "dw3000_mcps_mcu.h"
#include "dw3000_xtal_trim.h"
#include "task_signal.h"
#include "int_priority.h"

#include "minmax.h"
#include "deca_dbg.h"

extern struct dwchip_s *dwt_update_dw(struct dwchip_s *new_dw);

#ifndef dev_dbg
#define dev_dbg(a, b, c) diag_printf(b, c)
#endif

#ifndef CLAMP
#define CLAMP(x, min, max) (MIN(max, MAX(x, min)))
#endif

#define sts_to_pdoa(s) ((s) ? 3 : 0)

#define MCPS_TASK_TX_DONE 0x01
#define MCPS_TASK_RX_TIMEOUT 0x02
#define MCPS_TASK_RX_ERROR 0x04
#define MCPS_TASK_RX 0x08
#define MCPS_TASK_ALL                                               \
        (MCPS_TASK_RX | MCPS_TASK_RX_ERROR | MCPS_TASK_RX_TIMEOUT | \
         MCPS_TASK_TX_DONE)

#ifndef unlikely
#define unlikely(x) (0)
#endif

struct mcps_task_s {
        osThreadId Handle; /* Task's handler */
        osMutexId MutexId; /* Task's mutex */
};
typedef struct mcps_task_s mcps_task_t;

static mcps_task_t mcpsTask;

extern struct dwchip_s *dwt_update_dw(struct dwchip_s *new_dw);

static void McpsTask(void const *arg)
{
        struct mcps802154_llhw *local_llhw = (struct mcps802154_llhw *)arg;
        do {
                osMutexRelease(mcpsTask.MutexId);
                osEvent evt = osSignalWait(MCPS_TASK_ALL, osWaitForever);
                osMutexWait(
                        mcpsTask.MutexId,
                        0); //we do not want the task can be deleted in the middle of operation
                if (evt.value.signals & MCPS_TASK_TX_DONE) {
                        mcps802154_tx_done(local_llhw);
                }
                if (evt.value.signals & MCPS_TASK_RX_TIMEOUT) {
                        mcps802154_rx_timeout(local_llhw);
                }
                if (evt.value.signals & MCPS_TASK_RX_ERROR) {
                        mcps802154_rx_error(local_llhw,
                                            MCPS802154_RX_ERROR_OTHER);
                }
                if (evt.value.signals & MCPS_TASK_RX) {
                        mcps802154_rx_frame(local_llhw);
                }
        } while (1);
}

static void mcps_txdone_cb(const dwt_cb_data_t *txd)
{
        if (osSignalSet(mcpsTask.Handle, MCPS_TASK_TX_DONE) != osOK) {
                error_handler(1, _ERR_Signal_Bad);
        }
}

static void mcps_rxtimeout_cb(const dwt_cb_data_t *rxd)
{
        if (osSignalSet(mcpsTask.Handle, MCPS_TASK_RX_TIMEOUT) != osOK) {
                error_handler(1, _ERR_Signal_Bad);
        }
}

static void mcps_rxerror_cb(const dwt_cb_data_t *rxd)
{
        if (osSignalSet(mcpsTask.Handle, MCPS_TASK_RX_ERROR) != osOK) {
                error_handler(1, _ERR_Signal_Bad);
        }
}

/* @brief     ISR layer
 *             TWR application Rx callback
 *             to be called from dwt_isr() as an Rx call-back
 * */
static void mcps_rx_cb(const dwt_cb_data_t *rxd)
{
#define MAX_MSG 2
#define MAX_MSG_LEN 128
        static uint8_t message[MAX_MSG][MAX_MSG_LEN];
        static dwt_mcps_rx_t rx[MAX_MSG];
        static int idx = 0;

        struct dwchip_s *dw = rxd->dw;

        rx[idx].rtcTimeStamp = 0; //Not used
        {
           rx[idx].timeStamp = dw->dwt_driver->dwt_mcps_ops->get_timestamp(dw);
        }

        rx[idx].flags = 0;

        if (rxd->datalength) {
                rx[idx].len = MIN(rxd->datalength, MAX_MSG_LEN);
                rx[idx].data = (uint8_t *)&message[idx];
                struct dwt_rw_data_s rd = { (uint8_t *)rx[idx].data,
                                            rx[idx].len, 0 };
                dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_READRXDATA, 0, &rd);

                int16_t tmp;
                int     tmp1;
                dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_READCLOCKOFFSET, 0, (void *)&tmp);
                rx[idx].cfo = tmp;

                tmp1 = (int)( (float)tmp * (CLOCK_OFFSET_PPM_TO_RATIO * 1e6 * 100));                
                trim_XTAL_proc(dw, &dw->config->xtalTrim, tmp1);

        } else {
                // most likely an SP3 packet --> no data
                rx[idx].len = 0;
                rx[idx].data = NULL;
                rx[idx].flags |= DW3000_RX_FLAG_ND;
        }

#if (DIAG_READ_SUPPORT == 1)
        read_full_diagnostics(p, rxd->status);
#endif

        dw->rx = &rx[idx];

#if 0
    if (data->rx_flags & DW3000_CB_DATA_RX_FLAG_AAT)
            rx->flags |= DW3000_RX_FLAG_AACK;
#endif
        idx = (idx == MAX_MSG - 1) ? 0 : idx + 1;
        if (osSignalSet(mcpsTask.Handle, MCPS_TASK_RX) != osOK) {
                error_handler(1, _ERR_Signal_Bad);
        }
}

static int dw3000_setcallbacks(struct dwchip_s *dw)
{
        dw->callbacks.cbTxDone = mcps_txdone_cb;
        dw->callbacks.cbRxTo = mcps_rxtimeout_cb;
        dw->callbacks.cbRxErr = mcps_rxerror_cb;
        dw->callbacks.cbRxOk = mcps_rx_cb;
        return 0;
}

static inline u64 timestamp_dtu_to_rctu(struct mcps802154_llhw *llhw,
                                        u32 timestamp_dtu);

static inline u32 timestamp_rctu_to_dtu(struct mcps802154_llhw *llhw,
                                        u64 timestamp_rctu);

static inline void dw3000_update_timings(struct dwchip_s *dw);

static inline int dtu_to_pac(struct mcps802154_llhw *llhw, int timeout_dtu)
{
        struct dwchip_s *dw = llhw->priv;
        return (timeout_dtu * DW3000_CHIP_PER_DTU +
                dw->mcps_runtime->chips_per_pac - 1) /
               dw->mcps_runtime->chips_per_pac;
}

static inline int dtu_to_dly(struct mcps802154_llhw *llhw, int dtu)
{
        return (dtu * DW3000_CHIP_PER_DTU / DW3000_CHIP_PER_DLY);
}

static inline int rctu_to_dly(struct mcps802154_llhw *llhw, int rctu)
{
        return (rctu / DW3000_RCTU_PER_CHIP / DW3000_CHIP_PER_DLY);
}

static inline u32 tx_rmarker_offset(struct dwchip_s *dw, int ant_id)
{
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
        //struct dw3000_config *config = &dw->config;
        const struct dw3000_antenna_calib *ant_calib;
        const struct dw3000_antenna_calib_prf *ant_calib_prf;
        int chanidx;
        int prfidx;
#ifdef LATER
        if (ant_id >= ANTMAX || ant_id < 0) {
                dev_err(dw->dev, "ant_id %d is out of antenna range, max is %d",
                        ant_id, ANTMAX);
                return 0;
        }

        /* Current configured ant_id. */
        if (ant_id == config->ant[0])
                return config->rmarkerOffset;
#endif

        ant_calib = &dw->calib_data->ant[ant_id];

        chanidx = config->chan == 9 ? DW3000_CALIBRATION_CHANNEL_9 :
                                      DW3000_CALIBRATION_CHANNEL_5;
        prfidx = config->txCode >= 9 ? DW3000_CALIBRATION_PRF_64MHZ :
                                       DW3000_CALIBRATION_PRF_16MHZ;

        ant_calib_prf = &ant_calib->ch[chanidx].prf[prfidx];

        return ant_calib_prf->ant_delay;
}

//////////////////////////////////////////////////////////////////
static int start(struct mcps802154_llhw *llhw)
{
        struct dwchip_s *dw = llhw->priv;

        /* Configure antenna selection GPIO if any */
        int ret = dw3000_config_antenna_gpios(dw);
        if (unlikely(ret))
                return ret;

        /* Enable the device */
        return dw3000_enable(dw);
}

static void stop(struct mcps802154_llhw *llhw)
{
        struct dwchip_s *dw = llhw->priv;
        dw3000_disable(dw);
        return;
}


static int tx_frame(struct mcps802154_llhw *llhw, struct sk_buff *skb,
                    const struct mcps802154_tx_frame_info *info, int next_delay_dtu)
{
        struct dwchip_s *dw = llhw->priv;
        const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;
        u32 tx_date_dtu = 0;
        int rx_delay_dly = -1;
        u32 rx_timeout_pac = 0;
        int tx_delayed = 1;
        int rc;
        u8 sts_mode;

        /* Check data : no data if SP3, must have data otherwise */
        if (((info->flags & MCPS802154_TX_FRAME_STS_MODE_MASK) ==
             MCPS802154_TX_FRAME_SP3) != !skb)
                return -EINVAL;

        /* Enable STS */
        sts_mode = (info->flags & MCPS802154_TX_FRAME_STS_MODE_MASK) >> (__builtin_ffsll(MCPS802154_TX_FRAME_STS_MODE_MASK) - 1);
        rc = dw3000_set_sts(dw, sts_mode);

        if (unlikely(rc))
                return rc;
        rc = dw3000_setpdoa(dw, sts_to_pdoa(sts_mode));
        if (unlikely(rc))
                return rc;
        /* Ensure correct TX antenna is selected. */
        rc = dw3000_set_tx_antenna(dw, info->ant_id);
        if (unlikely(rc))
                return rc;
        /* Calculate the transfer date.*/
        if (info->flags & MCPS802154_TX_FRAME_TIMESTAMP_DTU) {
                tx_date_dtu = info->timestamp_dtu + llhw->shr_dtu;
        } else {
                /* Send immediately. */
                tx_delayed = 0;
        }

        if (info->rx_enable_after_tx_dtu > 0) {
                /* Disable auto-ack if it was previously enabled. */
                dw3000_disable_autoack(dw, false);
                /* Calculate the after tx rx delay. */
                rx_delay_dly = dtu_to_dly(llhw, info->rx_enable_after_tx_dtu) -
                               DW3000_RX_ENABLE_STARTUP_DLY;
                rx_delay_dly = rx_delay_dly >= 0 ? rx_delay_dly : 0;
                /* Calculate the after tx rx timeout. */
                if (info->rx_enable_after_tx_timeout_dtu == 0) {
                        rx_timeout_pac = dw->mcps_runtime->pre_timeout_pac;
                } else if (info->rx_enable_after_tx_timeout_dtu != -1) {
                        rx_timeout_pac =
                                dw->mcps_runtime->pre_timeout_pac +
                                dtu_to_pac(
                                        llhw,
                                        info->rx_enable_after_tx_timeout_dtu);
                } else {
                        /* No timeout. */
                }
        }
        int nok = dw3000_tx_frame(dw, skb, tx_delayed, tx_date_dtu,
                                  rx_delay_dly, rx_timeout_pac);
        /* TODO: need to specifically detect the delayed error. */
        if (nok) {
                uint8_t buf[4];
                ops->ioctl(dw, DWT_READSYSTIME, 0, &buf);
                u32 timestamp_dtu = (uint32_t)(buf[3] << 24 | buf[2] << 16 |
                                               buf[1] << 8 | buf[0]);
                diag_printf("Tx delayed: start %x current %x", tx_date_dtu,
                            timestamp_dtu);
                return -ETIME;
        }

        return 0;
}

static int rx_enable(struct mcps802154_llhw *llhw,
                     const struct mcps802154_rx_info *info, int next_delay_dtu)
{
        struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;

        const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

        u32 date_dtu = 0;
        u32 timeout_pac = 0;
        int rx_delayed = 1;
        int rc;
        u8 sts_mode;

        /* Enable STS */
        sts_mode = (info->flags & MCPS802154_RX_INFO_STS_MODE_MASK) >> (__builtin_ffsll(MCPS802154_RX_INFO_STS_MODE_MASK) - 1);
        rc = dw3000_set_sts(dw, sts_mode);
        if (unlikely(rc))
                return rc;
        rc = dw3000_setpdoa(dw, sts_to_pdoa(sts_mode));
        if (unlikely(rc))
                return rc;
        /* Ensure correct RX antenna are selected. */
        rc = dw3000_set_rx_antennas(dw, info->ant_pair_id);
        if (unlikely(rc))
                return rc;
        /* Calculate the transfer date. */
        if (info->flags & MCPS802154_RX_INFO_TIMESTAMP_DTU) {
                date_dtu = info->timestamp_dtu - DW3000_RX_ENABLE_STARTUP_DTU;
        } else {
                /* Receive immediately. */
                rx_delayed = 0;
        }

        if (info->flags & MCPS802154_RX_INFO_AACK) {
                dw3000_enable_autoack(dw, false);
        } else {
                dw3000_disable_autoack(dw, false);
        }

        /* Calculate the timeout. */
        if (info->timeout_dtu == 0) {
                timeout_pac = dw->mcps_runtime->pre_timeout_pac;
        } else if (info->timeout_dtu != -1) {
                timeout_pac = dw->mcps_runtime->pre_timeout_pac +
                              dtu_to_pac(llhw, info->timeout_dtu);
        } else {
                /* No timeout. */
        }
        /* TODO: need to specifically detect the delayed error. */
        int nok = dw3000_rx_enable(dw, rx_delayed, date_dtu, timeout_pac);
        if (nok) {
                uint8_t buf[4];
                ops->ioctl(dw, DWT_READSYSTIME, 0, &buf);
                u32 timestamp_dtu = (uint32_t)(buf[3] << 24 | buf[2] << 16 |
                                               buf[1] << 8 | buf[0]);
                diag_printf("Rx delayed: start %x current %x", date_dtu,
                            timestamp_dtu);
                return -ETIME;
        }
        return 0;
}

static int rx_disable(struct mcps802154_llhw *llhw)
{
        struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
        return dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_FORCETRXOFF, 0, 0);
}

/**
 * get_ranging_pdoa_fom() - compute the figure of merit of the PDoA.
 * @sts_fom: STS FoM on a received frame.
 *
 * If the STS FoM is less than sts_fom_threshold, PDoA FoM is 1, the worst.
 * If the STS FoM is greater or equal than sts_fom_threshold,
 * sts_fom_threshold to 255 values are mapped to 2 to 255.
 *
 * Return: the PDoA FoM value.
 */
static u8 get_ranging_pdoa_fom(u8 sts_fom)
{
        /* For a normalized STS FoM in 0 to 255, the STS is not reliable if
         * the STS FoM is less than 60 percents of its maximum value.
         */
        static const int sts_fom_threshold = 153;
        /* sts_fom_threshold .. sts_fom_max values are mapped to pdoa_fom_min .. pdoa_fom_max.
         * The relation is pdoa_fom = a * sts_fom + b, with
         * pdoa_fom_min =  sts_fom_threshold * a + b
         * pdoa_fom_max = sts_fom_max * a + b
         * So:
         * a = (pdoa_fom_max - pdoa_fom_min) / (sts_fom_max - sts_fom_threshold)
         * b = pdoa_fom_min - sts_fom_threshold * a
         */
        const int sts_fom_max = 255;
        const int pdoa_fom_min = 2;
        const int pdoa_fom_max = 255;
        const int a_numerator = pdoa_fom_max - pdoa_fom_min;
        const int a_denominator = sts_fom_max - sts_fom_threshold;
        const int b = pdoa_fom_min - ((sts_fom_threshold * a_numerator) /
                                             a_denominator);

        if (sts_fom < sts_fom_threshold)
                return 1;
        return ((a_numerator * sts_fom) / a_denominator) + b;
}

static int get_ranging_sts_fom(struct mcps802154_llhw *llhw,
                               struct mcps802154_rx_frame_info *info)
{
        struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
        const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;
        /* Max sts_acc_qual value depend on STS length */
        int sts_acc_max = 32 << config->stsLength;
        s16 sts_acc_qual;
        /* TODO: Reading TOAST disabled. According to hardware team,
         * this needs more tuning. They suggest to use quality only for
         * now. See UWB-940 and commit "disable TOAST quality checking
         * for STS". */

        ops->ioctl(dw, DWT_READSTSQUALITY, 0, (void *)&sts_acc_qual);

        /* DW3000 only support one STS segment. */
        info->ranging_sts_fom[0] =
                (uint8_t)(CLAMP(1 + sts_acc_qual * 254 / sts_acc_max, 1, 255));

        return 0;
}

static int rx_get_frame(struct mcps802154_llhw *llhw, struct sk_buff **skb,
                        struct mcps802154_rx_frame_info *info)
{
        struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
        struct dwt_mcps_rx_s *rx = dw->rx;
        int ret = 0;

        /* Sanity check parameters */
        if (unlikely(!info || !skb)) {
                ret = -EINVAL;
                goto error;
        }

        struct sk_buff *local_skb = malloc(sizeof(struct sk_buff));
        *skb = local_skb;

        /* Check buffer available */
        if (unlikely(!*skb && !(rx->flags & DW3000_RX_FLAG_ND))) {
                //              spin_unlock_irqrestore(&rx->lock, flags);
                ret = -EAGAIN;
                goto error;
        }
        local_skb->data = dw->rx->data;
        local_skb->len = dw->rx->len;
        if (local_skb->len)
                local_skb->len -= IEEE802154_FCS_LEN;

        if (info->flags & (MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU |
                           MCPS802154_RX_FRAME_INFO_TIMESTAMP_DTU)) {
                info->timestamp_rctu =
                        rx->timeStamp - dw->config->rxtx_config->rxAntDelay;
                info->timestamp_dtu =
                        timestamp_rctu_to_dtu(llhw, rx->timeStamp) -
                        llhw->shr_dtu;
        }

        /* In case of auto-ack send. */
        if (rx->flags & DW3000_RX_FLAG_AACK) {
                info->flags |= MCPS802154_RX_FRAME_INFO_AACK;
        }

        /* In case of PDoA. */
        if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_PDOA) {
                info->ranging_pdoa_rad_q11 = dw3000_readpdoa(dw);
                info->ranging_pdoa_spacing_mm_q11 =
                        DW3000_DEFAULT_ANTPAIR_SPACING;
        }

        /* In case of STS */
        if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_STS_TIMESTAMP_RCTU) {
                u64 sts_ts_rctu;

                ret = 0;
                sts_ts_rctu = rx->timeStamp;

                if (ret)
                        goto error;
                /* DW3000 only support one STS segment. */
                info->ranging_sts_timestamp_diffs_rctu[0] = 0;
                info->ranging_sts_timestamp_diffs_rctu[1] =
                        sts_ts_rctu - rx->timeStamp;
        }

        info->ranging_sts_fom[0] = 0;
        if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM) {
                ret = get_ranging_sts_fom(llhw, info);
                if (ret) {
                        goto error;
                }
        }

        if (info->flags & MCPS802154_RX_FRAME_INFO_RANGING_PDOA_FOM) {
                if (!(info->flags & MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM)) {
                        ret = get_ranging_sts_fom(llhw, info);
                        if (ret) {
                                goto error;
                        }
                }
                if (info->ranging_sts_fom[0]) {
                        info->ranging_pdoa_fom =
                                get_ranging_pdoa_fom(info->ranging_sts_fom[0]);
                } else {
                        info->ranging_pdoa_fom = 0;
                }
        }

        /* Keep only implemented. */
        info->flags &= (MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU |
                        MCPS802154_RX_FRAME_INFO_TIMESTAMP_DTU |
                        MCPS802154_RX_FRAME_INFO_AACK |
                        MCPS802154_RX_FRAME_INFO_RANGING_PDOA |
                        MCPS802154_RX_FRAME_INFO_RANGING_PDOA_FOM |
                        MCPS802154_RX_FRAME_INFO_RANGING_STS_FOM |
                        MCPS802154_RX_FRAME_INFO_RANGING_STS_TIMESTAMP_RCTU);

        /* Check for the frame ranging , SP3 */
        if ((*skb)->len == 0) {
                free(*skb);
                *skb = NULL;
        }

error:
        return ret;
}

static int rx_get_error_frame(struct mcps802154_llhw *llhw,
                              struct mcps802154_rx_frame_info *info)
{
        int ret = 0;

        /* Sanity check */
        if (info == NULL) {
                ret = -1;
                goto error;
        }
        if (info->flags & MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU) {
#ifdef LATER
                              if (dw3000_read_rx_timestamp(dw, &info->timestamp_rctu))
                                      info->flags &= ~MCPS802154_RX_FRAME_INFO_TIMESTAMP_RCTU;
#endif
        } else {
                /* Not implemented */
                info->flags = 0;
        }
error:
        return ret;
}


static int reset(struct mcps802154_llhw *llhw)
{
        struct dwchip_s *dw = llhw->priv;
        const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

        int ret = 0;

        port_disable_dw_irq_and_reset(dw->config->do_reset);

        taskENTER_CRITICAL(); /**< When the app will setup RTOS tasks, then if task has a higher priority, the kernel will start it immediately, thus we need to stop the scheduler.*/

        dw->SPI->setfastrate(); //restore fast mode after reset

        /* dwt_xx calls in app level Must be in protected mode (DW3000 IRQ disabled) */
        disable_dw3000_irq();

        dw3000_setcallbacks(dw);

        /* Set time related field that are configuration dependent */
        dw3000_update_timings(dw);

        /* Need to make sure DW IC is in IDLE_RC before proceeding */
        while (!dwt_checkidlerc())
                ;
        if (ops->init(dw)) /**< set callbacks to NULL inside dwt_initialise*/
        {
                diag_printf("Tag init failed. Stop.\r\n");
                ret = _ERR_INIT; // device initialise has failed
        }

        { //MCPS adjust Antenna delays manually 
          int16_t tmp = 0;
          dw->dwt_driver->dwt_ops->ioctl(dw, DWT_SETTXANTENNADELAY, 0, (void *)&tmp);
          dw->dwt_driver->dwt_ops->ioctl(dw, DWT_SETRXANTENNADELAY, 0, (void *)&tmp);
        }

        taskEXIT_CRITICAL(); /**< all RTOS tasks can be scheduled */

        return ret;
}

static int get_current_timestamp_dtu(struct mcps802154_llhw *llhw,
                                     u32 *timestamp_dtu)
{
        struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
        const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

#define DW3000_SPI_COLLISION_STATUS_BIT_MASK 0x1fU
        uint32_t data = DW3000_SPI_COLLISION_STATUS_BIT_MASK;
        dw->dwt_driver->dwt_ops->ioctl(dw, DWT_WRITE_REG, 0x10020,
                                       (void *)data);

        uint8_t buf[4];
        ops->ioctl(dw, DWT_READSYSTIME, 0, &buf);
        *timestamp_dtu =
                (uint32_t)(buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0]);
        return 0;
}

static inline u64 timestamp_dtu_to_rctu(struct mcps802154_llhw *llhw,
                                        u32 timestamp_dtu)
{
        return ((u64)timestamp_dtu) * DW3000_RCTU_PER_DTU;
}

static inline u32 timestamp_rctu_to_dtu(struct mcps802154_llhw *llhw,
                                        u64 timestamp_rctu)
{
        return (u32)(timestamp_rctu / DW3000_RCTU_PER_DTU);
}

static inline u64 tx_timestamp_dtu_to_rmarker_rctu(struct mcps802154_llhw *llhw,
                                                   u32 tx_timestamp_dtu,
                                                   int ant_id)
{
        struct dwchip_s *dw = (struct dwchip_s *)llhw->priv;
        /* LSB is ignored. */
        const u32 bit_mask = ~1;
        const u64 rctu = timestamp_dtu_to_rctu(
                llhw, (tx_timestamp_dtu + llhw->shr_dtu) & bit_mask);

        return (rctu + dw->config->rxtx_config->txAntDelay);

}

static inline s64 difference_timestamp_rctu(struct mcps802154_llhw *llhw,
                                            u64 timestamp_a_rctu,
                                            u64 timestamp_b_rctu)
{
        const u64 rctu_rollover = 1ll << 40;
        const u64 rctu_mask = rctu_rollover - 1;
        s64 diff_rctu = (timestamp_a_rctu - timestamp_b_rctu) & rctu_mask;
        if (diff_rctu & (rctu_rollover >> 1))
                diff_rctu = diff_rctu | ~rctu_mask;
        return diff_rctu;
}

static int compute_frame_duration_dtu(struct mcps802154_llhw *llhw,
                                      int payload_bytes)
{
        struct dwchip_s *dw = llhw->priv;
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
        int chip_per_symb, phr_chip_per_symb, data_chip_per_symb;

        if (config->txCode >= 9)
                chip_per_symb = 508; //@64MHz
        else
                chip_per_symb = 496; //@16MHz

        if (config->dataRate == DWT_BR_850K) {
                phr_chip_per_symb = 512;
                data_chip_per_symb = 512;
        } else {
                phr_chip_per_symb = 512;
                data_chip_per_symb = 64;
        }

        /* STS part */
        const int sts_symb = config->stsMode == DWT_STS_MODE_OFF ?
                                     0 :
                                     32 << config->stsLength;
        const int sts_chips = sts_symb * chip_per_symb;

        /* PHR part. */
        static const int phr_tail_bits = 19 + 2;
        const int phr_chips = phr_tail_bits * phr_chip_per_symb; // 1 bit/symbol

        // Data part, 48 Reed-Solomon bits per 330 bits.
        const int data_bits = payload_bytes * 8;
        const int data_rs_bits = data_bits + (data_bits + 329) / 330 * 48;
        const int data_chips =
                data_rs_bits * data_chip_per_symb; // 1 bit/symbol

        // Done, convert to dtu.
        //dev_dbg(dw->dev, "%s called\n", __func__);
        return llhw->shr_dtu +
               (sts_chips + phr_chips + data_chips) / DW3000_CHIP_PER_DTU;
}

static inline int dw3000_compute_shr_dtu(struct dwchip_s *dw)
{
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

        const int symb = deca_to_plen(config->txPreambLength);
        const int sfd_symb =
                (config->dataRate == 0) && (config->sfdType == 1) ? 16 : 8;
        const int shr_symb = symb + sfd_symb;

        const int chip_per_symb = config->txCode >= 9 ? 508 : 496;

        return shr_symb * chip_per_symb / DW3000_CHIP_PER_DTU;
}

static inline int dw3000_compute_symbol_dtu(struct dwchip_s *dw)
{
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

        const int chip_per_symb = config->txCode >= 9 ? 508 : 496;
        return chip_per_symb / DW3000_CHIP_PER_DTU;
}

static inline int dw3000_compute_chips_per_pac(struct dwchip_s *dw)
{
        //PAC means preamble accumulation count
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
        int symb = deca_to_plen(config->txPreambLength);
        int pac_symb;

        if (symb <= 128) {
                pac_symb = 8;
        }
        if (symb <= 512) {
                pac_symb = 16;
        }
        if (symb <= 1024) {
                pac_symb = 32;
        } else {
                pac_symb = 64;
        }
        const int chip_per_symb = config->txCode >= 9 ? 508 : 496;

        return chip_per_symb * pac_symb;
}

static inline int dw3000_compute_pre_timeout_pac(struct dwchip_s *dw)
{
        /* Must be called AFTER dw->chips_per_pac initialisation */
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;
        int symb = deca_to_plen(config->txPreambLength);
        int pac_symb;

        if (symb <= 128) {
                pac_symb = 8;
        }
        if (symb <= 512) {
                pac_symb = 16;
        }
        if (symb <= 1024) {
                pac_symb = 32;
        } else {
                pac_symb = 64;
        }

        return (DW3000_RX_ENABLE_STARTUP_DLY * DW3000_CHIP_PER_DLY +
                dw->mcps_runtime->chips_per_pac - 1) /
                       dw->mcps_runtime->chips_per_pac +
               symb / pac_symb + 2;
}

static inline void dw3000_update_timings(struct dwchip_s *dw)
{
        struct mcps802154_llhw *llhw = dw->llhw;
        /* Update configuration dependant timings */
        llhw->shr_dtu = dw3000_compute_shr_dtu(dw);
        llhw->symbol_dtu = dw3000_compute_symbol_dtu(dw);
        /* The CCA detection time shall be equivalent to 40 data symbol periods,
       Tdsym, for a nominal 850 kb/s, or equivalently, at least 8 (multiplexed)
       preamble symbols should be captured in the CCA detection time. */

        llhw->cca_dtu = 8 * llhw->symbol_dtu;
        dw->mcps_runtime->chips_per_pac = dw3000_compute_chips_per_pac(dw);
        dw->mcps_runtime->pre_timeout_pac = dw3000_compute_pre_timeout_pac(dw);
}

static int set_channel(struct mcps802154_llhw *llhw, u8 page, u8 channel,
                       u8 preamble_code)
{
        struct dwchip_s *dw = llhw->priv;
        dwt_config_t *config = dw->config->rxtx_config->pdwCfg;

        int error = 0;
        /* Check parameters early */
        if (page != 4 || (channel != 5 && channel != 9)) {
                error = -1;
        }
        if (!error) {
                switch (preamble_code) {
                /* Set default value if MCPS don't give one to driver */
                case 0:
                        preamble_code = 9;
                        break;
                /* DW3000_PRF_16M */
                case 3:
                case 4:
                /* DW3000_PRF_64M */
                case 9:
                case 10:
                case 11:
                case 12:
                        break;
                default:
                        error = -1;
                }
        }
        if (!error) {
                config->chan = channel;
                config->txCode = preamble_code;
                config->rxCode = preamble_code;
        }
        const struct dwt_mcps_ops_s *dwt_mcps_ops =
                dw->dwt_driver->dwt_mcps_ops;
        if (dwt_mcps_ops->set_channel != NULL) {
                error = dwt_mcps_ops->set_channel(dw, page, channel,
                                                  preamble_code);
        }
        return error;
}

static int set_hrp_uwb_params(struct mcps802154_llhw *llhw, int prf, int psr,
                              int sfd_selector, int phr_rate, int data_rate)
{
        //struct dw3000 *dw = llhw->priv;
        //dev_dbg(dw->dev, "%s called\n", __func__);
        return 0;
}

/**
 * @brief src(array) -> dst(uint32_t)
 * @src: *array16 bytes
 * @dst: *uint32_t[4]
 */
static inline void swap16(uint32_t *dst, const uint8_t *src)
{
        dst[3] = src[0] << 24 | src[1] << 16 | src[2] << 8 | src[3];
        dst[2] = src[4] << 24 | src[5] << 16 | src[6] << 8 | src[7];
        dst[1] = src[8] << 24 | src[9] << 16 | src[10] << 8 | src[11];
        dst[0] = src[12] << 24 | src[13] << 16 | src[14] << 8 | src[15];
}

static int set_sts_params(struct mcps802154_llhw *llhw,
                          const struct mcps802154_sts_params *arg)
{
        struct dwchip_s *dw = llhw->priv;
        const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

        int tmp, rc;
        uint32_t tmp32[4];
        dwt_sts_lengths_e len;

        tmp = (arg->seg_len == 32) ?
                      (DWT_STS_LEN_32) :
                      (arg->seg_len == 64) ?
                      (DWT_STS_LEN_64) :
                      (arg->seg_len == 128) ?
                      (DWT_STS_LEN_128) :
                      (arg->seg_len == 256) ?
                      (DWT_STS_LEN_256) :
                      (arg->seg_len == 512) ?
                      (DWT_STS_LEN_512) :
                      (arg->seg_len == 1024) ?
                      (DWT_STS_LEN_1024) :
                      (arg->seg_len == 2048) ? (DWT_STS_LEN_2048) : (-1);

        if (tmp < 0) {
                return -EINVAL;
        }

        len = (dwt_sts_lengths_e)tmp;

        /* Check parameters */
        if (arg->n_segs > 1) {
                return -EINVAL;
        }
        /* Set STS segment(s) length */
        rc = dw3000_set_sts_length(dw, len);

        if (rc) {
                return rc;
        }
        /* Send KEY & IV */
        swap16(tmp32, arg->key);
        rc = ops->ioctl(dw, DWT_CONFIGURESTSKEY, 0, (void *)tmp32);
        if (rc) {
                return rc;
        }

        swap16(tmp32, arg->v);
        rc = ops->ioctl(dw, DWT_CONFIGURESTSIV, 0, (void *)tmp32);
        if (rc) {
                return rc;
        }
        return ops->ioctl(dw, DWT_CONFIGURESTSLOADIV, 0, NULL);
}

static int set_hw_addr_filt(struct mcps802154_llhw *llhw,
                            struct ieee802154_hw_addr_filt *filt,
                            unsigned long changed)
{
        struct dwchip_s *dw = llhw->priv;
        const struct dwt_mcps_ops_s *ops = dw->dwt_driver->dwt_mcps_ops;

        int error = 0;

        if (changed & IEEE802154_AFILT_SADDR_CHANGED) {
                error = ops->ioctl(dw, DWT_SETADDRESS16, 0,
                                   (void *)&filt->short_addr);
        }
        if (!error && (changed & IEEE802154_AFILT_IEEEADDR_CHANGED)) {
                error = ops->ioctl(dw, DWT_SETEUI, 0, (void *)&filt->ieee_addr);
        }
        if (!error && (changed & IEEE802154_AFILT_PANID_CHANGED)) {
                error = ops->ioctl(dw, DWT_SETPANID, 0, (void *)&filt->pan_id);
        }
        if (!error && (changed & IEEE802154_AFILT_PANC_CHANGED)) {
                if (filt->pan_coord) {
                        struct dwt_configure_ff_s tmp = { 1, DWT_FF_COORD_EN };
                        error = ops->ioctl(dw, DWT_CONFIGUREFRAMEFILTER, 0,
                                           (void *)&tmp);
                } else {
                        struct dwt_configure_ff_s tmp = { 0, 0 };
                        error = ops->ioctl(dw, DWT_CONFIGUREFRAMEFILTER, 0,
                                           (void *)&tmp);
                }
        }
        return error;
}

static int set_txpower(struct mcps802154_llhw *llhw, s32 mbm)
{
        //struct dw3000 *dw = llhw->priv;
        //dev_dbg(dw->dev, "%s called\n", __func__);
        return 0;
}

static int set_cca_mode(struct mcps802154_llhw *llhw,
                        const struct wpan_phy_cca *cca)
{
        return 0;
}

static int set_cca_ed_level(struct mcps802154_llhw *llhw, s32 mbm)
{
        return 0;
}

static int set_promiscuous_mode(struct mcps802154_llhw *llhw, bool on)
{
        struct dwchip_s *dw = llhw->priv;
        rxtx_configure_t *p = dw->config->rxtx_config;
        const struct dwt_mcps_ops_s *dwt_mcps_ops =
                dw->dwt_driver->dwt_mcps_ops;

        p->frameFilter = on ? 1 : 0;
        p->frameFilterMode = DWT_FF_BEACON_EN | DWT_FF_DATA_EN | DWT_FF_ACK_EN |
                             DWT_FF_COORD_EN;

        struct dwt_configure_ff_s tmp = { p->frameFilter, p->frameFilterMode };

        dw->dwt_driver->dwt_mcps_ops->ioctl(dw, DWT_CONFIGUREFRAMEFILTER, 0,
                                            (void *)&tmp);
        return 0;
}

static int set_calibration(struct mcps802154_llhw *llhw, const char *key,
                           void *value, size_t length)
{
        struct dwchip_s *dw = llhw->priv;
        void *param;
        int len;
        /* Sanity checks */
        if (!key || !value || !length)
                return -1;
        /* The individual calibration data can be storred in the OTP of the DW3000 chip during mass production.
         * With a good approximation a mean default value can be used instead.
         * OTP calibration is not used in this example
         */
        return 0;
}

static int get_calibration(struct mcps802154_llhw *llhw, const char *key,
                           void *value, size_t length)
{
        struct dwchip_s *dw = llhw->priv;
        void *param;
        int len = 0; 
        /* Sanity checks */
        if (!key)
                return -1;

        /* The individual calibration data can be storred in the OTP of the DW3000 chip during mass production.
         * With a good approximation a mean default value can be used instead.
         * OTP calibration is not used in this example
         */

        return len;
}

static const char *const *list_calibration(struct mcps802154_llhw *llhw)
{
        /* The individual calibration data can be storred in the OTP of the DW3000 chip during mass production.
         * With a good approximation a mean default value can be used instead.
         * OTP calibration is not used in this example
         */

        return NULL;
}

/**
 * vendor_cmd() - Forward vendor commands processing to dw3000 function.
 *
 * @llhw: Low-level hardware without MCPS.
 * @vendor_id: Vendor Identifier on 3 bytes.
 * @subcmd: Sub-command identifier.
 * @data: Null or data related with the sub-command.
 * @data_len: Length of the data
 *
 * Return: 0 on success, 1 to request a stop, error on other value.
 */
static int vendor_cmd(struct mcps802154_llhw *llhw, u32 vendor_id, u32 subcmd,
                      void *data, size_t data_len)
{
#ifdef LATER
        struct dwchip_s *dw = llhw->priv;

        /* There is only NFCC coexitence vendor command for now. */
        return dw3000_nfcc_coex_vendor_cmd(dw, vendor_id, subcmd, data,
                                           data_len);
#else
        return 0;
#endif
        return -1;
}

const struct mcps802154_ops mcps_mcu_ops = {
        .start = start,
        .stop = stop,
        .tx_frame = tx_frame,
        .rx_enable = rx_enable,
        .rx_disable = rx_disable,
        .rx_get_frame = rx_get_frame,
        .rx_get_error_frame = rx_get_error_frame,
        .reset = reset,
        .get_current_timestamp_dtu = get_current_timestamp_dtu,
        .timestamp_dtu_to_rctu = timestamp_dtu_to_rctu,
        .timestamp_rctu_to_dtu = timestamp_rctu_to_dtu,
        .tx_timestamp_dtu_to_rmarker_rctu = tx_timestamp_dtu_to_rmarker_rctu,
        .difference_timestamp_rctu = difference_timestamp_rctu,
        .compute_frame_duration_dtu = compute_frame_duration_dtu,
        .set_channel = set_channel,
        .set_hrp_uwb_params = set_hrp_uwb_params,
        .set_sts_params = set_sts_params,
        .set_hw_addr_filt = set_hw_addr_filt,
        .set_txpower = set_txpower,
        .set_cca_mode = set_cca_mode,
        .set_cca_ed_level = set_cca_ed_level,
        .set_promiscuous_mode = set_promiscuous_mode,
        .set_calibration = set_calibration,
        .get_calibration = get_calibration,
        .list_calibration = list_calibration,
        .vendor_cmd = vendor_cmd,
        MCPS802154_TESTMODE_CMD(dw3000_tm_cmd)
};

/*
 * probe of the DW chip
 */
static bool dw_chip_alloc(struct dwchip_s *dw)
{
        uint32_t devId;
        uint8_t addr, buf[sizeof(uint32_t)];

        if (!(dw->calib_data =
                      malloc(sizeof(struct dw3000_calibration_data)))) {
                return false;
        }

        if (!(dw->mcps_runtime = malloc(sizeof(struct dwt_mcps_runtime_s)))) {
                free(dw->calib_data);
                return false;
        }

        //initialise the crc calculation lookup table
        //_dwt_crc8init();

        dw->SPI = (struct dwt_spi_s *) dw3000_probe_interf.spi;
        dw->priv = NULL;
        dw->coex_gpio_pin = -1;

        //Device ID address is common in between all DW chips
        addr = 0x00; //DW3XXX_DEVICE_ID;

        dw->SPI->readfromspi(sizeof(uint8_t), &addr, sizeof(buf), buf);

        devId = buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];

        /* Linker file shall have a section with a list of driver descriptors*/
        extern uint32_t __dw_drivers_start;
        extern uint32_t __dw_drivers_end;

        struct dwt_driver_s *drv;

        dw->dwt_driver = NULL;

        for (drv = (struct dwt_driver_s *)&__dw_drivers_start;
             drv < (struct dwt_driver_s *)&__dw_drivers_end; drv++) {
                if ((devId & drv->devmatch) == (drv->devid & drv->devmatch)) {
                        dw->dwt_driver = drv;
                }
        }

        if (!dw->dwt_driver) {
                free(dw->mcps_runtime);
                free(dw->calib_data);
                return false;
        }

        return true;
}

static void dw_chip_free(struct dwchip_s *dw)
{
        if (!dw)
                return;
        free(dw->priv);
        free(dw->mcps_runtime);
        free(dw->calib_data);
}

void dw3000_mcps_free(struct dwchip_s *dw)
{
        if (dw) {
                dw_chip_free(dw); //this will free priv, SPI and Queue, NOT dw
                mcps802154_free_llhw(
                        dw->llhw); //will also free dw since it has been allocated by mcps802154_alloc_llhw()
                dw = NULL;
        }
}

struct dwchip_s *dw3000_mcps_alloc(void)
{
        struct mcps802154_llhw *llhw;
        struct dwchip_s *dw;

        llhw = mcps802154_alloc_llhw(sizeof(struct dwchip_s), &mcps_mcu_ops);
        if (llhw == NULL)
                return NULL;

        dw = llhw->priv;
        dw->llhw = llhw;
        dw->mcps_ops = (struct mcps802154_ops *)&mcps_mcu_ops;

        if (!dw_chip_alloc(dw)) {
                dw3000_mcps_free(dw);
                return NULL;
        }

        //      dw3000_init_config(dw);

        /* Configure IEEE802154 HW capabilities */
        llhw->hw->flags =
                (IEEE802154_HW_TX_OMIT_CKSUM | IEEE802154_HW_AFILT |
                 IEEE802154_HW_PROMISCUOUS | IEEE802154_HW_RX_OMIT_CKSUM);
        llhw->flags = llhw->hw->flags;

#ifdef LATER
        /* UWB High band 802.15.4a-2007. Only channels 5 & 9 for DW3000. */
        llhw->hw->phy->supported.channels[4] = (1 << 5) | (1 << 9);
#endif
        /* Set time related fields */
        llhw->dtu_freq_hz = DW3000_DTU_FREQ;
        llhw->dtu_rctu = DW3000_RCTU_PER_DTU;
        llhw->rstu_dtu = DW3000_DTU_PER_RSTU;
        llhw->anticip_dtu = 16 * (DW3000_DTU_FREQ / 1000);

        /* Set extended address. */
        llhw->hw->phy->perm_extended_addr = 0xd6552cd6e41ceb57;

#ifdef LATER
        /* Symbol is ~0.994us @ PRF16 or ~1.018us @ PRF64. Use 1. */
        llhw->hw->phy->symbol_duration = 1;


        /* Driver phy channel 5 on page 4 as default */
        llhw->hw->phy->current_channel = 5;
        llhw->hw->phy->current_page = 4;
#endif
        return dw;
}

static struct dwchip_s *old_dw = NULL;
int dw3000_mcps_register(struct dwchip_s *dw)
{
        int r = mcps802154_register_llhw(dw->llhw);

        //create mutex before the task
        osMutexDef(mcpsMutex);
        mcpsTask.MutexId = osMutexCreate(osMutex(mcpsMutex));
#ifndef __ZEPHYR__
        osThreadDef(mcpsTask, McpsTask, osPriorityRealtime, 0, 1024);
        mcpsTask.Handle = osThreadCreate(osThread(mcpsTask), dw->llhw);
#else
        osThreadDef(McpsTask, osPriorityRealtime, 0, 512);
        mcpsTask.Handle = osThreadCreate(osThread(McpsTask), dw->llhw);
#endif

        reset(dw->llhw);
        old_dw = dwt_update_dw(dw);
        return r;
}

void dw3000_mcps_unregister(struct dwchip_s *dw)
{
        TERMINATE_STD_TASK(mcpsTask);
        mcps802154_unregister_llhw(dw->llhw);
        dwt_update_dw(old_dw);
}
