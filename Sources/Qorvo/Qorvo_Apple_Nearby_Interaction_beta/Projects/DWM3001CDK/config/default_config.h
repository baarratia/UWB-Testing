/**
 * @file      default_config.h
 *
 * @brief     Default config file for NVM initialization
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef __DEFAULT_CONFIG_H__H__
#define __DEFAULT_CONFIG_H__H__ 1

#ifdef __cplusplus
 extern "C" {
#endif

#include "niq.h"
#include "common_fira.h"
#include "fira_region_params.h"
#include "deca_device_api.h"

/* UWB config */
#define DEFAULT_CHANNEL             (9)
#define DEFAULT_TXPREAMBLENGTH      (DWT_PLEN_64)
#define DEFAULT_RXPAC               (DWT_PAC8)
#define DEFAULT_PCODE               (11)
#define DEFAULT_NSSFD               (DWT_SFD_IEEE_4Z)
#define DEFAULT_DATARATE            (DWT_BR_6M8)
#define DEFAULT_PHRMODE             (DWT_PHRMODE_STD)
#define DEFAULT_PHRRATE             (DWT_PHRRATE_STD)
#define DEFAULT_SFDTO               (64 + 1 + 8 - 8)
#define DEFAULT_STS_MODE            (DWT_STS_MODE_OFF)
#define DEFAULT_STS_LENGTH          (DWT_STS_LEN_64)
#define DEFAULT_PDOA_MODE           (DWT_PDOA_M0)

/* run-time config */
#define DEFAULT_ANTD                (513.484f * 1e-9 / DWT_TIME_UNITS) /* Mean Total Antenna Delay for DWM3001C module */
#define DEFAULT_RNGOFF_MM           (0) /* Can be used to offset Range */
#define DEFAULT_REPORT_LEVEL        (1)

#define FCONFIG_SIZE                (0x200)

/* Default configuration initialization */
#define DEFAULT_CONFIG \
{\
    .dwt_config.chan            = DEFAULT_CHANNEL, \
    .dwt_config.txPreambLength  = DEFAULT_TXPREAMBLENGTH, \
    .dwt_config.rxPAC           = DEFAULT_RXPAC, \
    .dwt_config.txCode          = DEFAULT_PCODE, \
    .dwt_config.rxCode          = DEFAULT_PCODE, \
    .dwt_config.sfdType         = DEFAULT_NSSFD, \
    .dwt_config.dataRate        = DEFAULT_DATARATE, \
    .dwt_config.phrMode         = DEFAULT_PHRMODE, \
    .dwt_config.phrRate         = DEFAULT_PHRRATE, \
    .dwt_config.sfdTO           = DEFAULT_SFDTO, \
    .dwt_config.stsMode         = DEFAULT_STS_MODE, \
    .dwt_config.stsLength       = DEFAULT_STS_LENGTH, \
    .dwt_config.pdoaMode        = DEFAULT_PDOA_MODE, \
    \
    .v.ver0                     = 0xFFFFFFFF, \
    .v.ver1                     = 0xFFFFFFFF, \
    .v.ver2                     = 0xFFFFFFFF, \
    .v.ver3                     = 0xFFFFFFFF, \
    \
    .s.last_error = 0,\
    .s.mode = 0,\
    \
    .s.txConfig.PGdly           = 0x34, \
    .s.txConfig.power           = 0xfefefefeUL, \
    .s.txConfig.PGcount         = 0, \
    \
    .s.rngOffset_mm         = DEFAULT_RNGOFF_MM, \
    .s.reportLevel          = DEFAULT_REPORT_LEVEL, \
    \
    .s.antRx_a              = (uint16_t)(0.5* DEFAULT_ANTD), \
    .s.antTx_a              = (uint16_t)(0.5* DEFAULT_ANTD), \
    \
    .s.xtalTrim             = 0x2E,\
    \
    .s.fira_config. role = 0,\
    .s.fira_config. enc_payload = 1,\
    .s.fira_config. Session_ID = 1111,\
    .s.fira_config. Ranging_Round_Usage = FIRA_RANGING_ROUND_USAGE_DSTWR,\
    .s.fira_config. Multi_Node_Mode = FIRA_MULTI_NODE_MODE_UNICAST,\
    .s.fira_config. Rframe_Config = FIRA_RFRAME_CONFIG_SP3,\
    .s.fira_config. ToF_Report = 1,\
    .s.fira_config. AoA_Azimuth_Report = 0,\
    .s.fira_config. AoA_Elevation_Report = 0,\
    .s.fira_config. AoA_FOM_Report = 0,\
    .s.fira_config. nonDeferred_Mode = 0,\
    .s.fira_config. STS_Config = 0,\
    .s.fira_config. Round_Hopping = 0,\
    .s.fira_config. Block_Striding = 0,\
    .s.fira_config. Block_Duration_ms = 100,\
    .s.fira_config. Round_Duration_RSTU = 18400,\
    .s.fira_config. Slot_Duration_RSTU = 2400,\
    .s.fira_config. Channel_Number = 9,\
    .s.fira_config. Preamble_Code = 11,\
    .s.fira_config. PRF_Mode = 0,\
    .s.fira_config. SP0_PHY_Set = BPRF_SET_2,\
    .s.fira_config. SP1_PHY_Set = BPRF_SET_3,\
    .s.fira_config. SP3_PHY_Set = BPRF_SET_4,\
    .s.fira_config. MAX_RR_Retry = 1,\
    .s.fira_config. Constraint_Length_Conv_Code_HPRF = 0,\
    .s.fira_config. UWB_Init_Time_ms = 5,\
    .s.fira_config. Block_Timing_Stability = 0,\
    .s.fira_config. Key_Rotation = 0,\
    .s.fira_config. Key_Rotation_Rate = 0,\
    .s.fira_config. MAC_FCS_TYPE = 0,\
    .s.fira_config. MAC_ADDRESS_MODE = 0,\
    .s.fira_config. SRC_ADDR[0] = 0,\
    .s.fira_config. SRC_ADDR[1] = 0,\
    .s.fira_config. Number_of_Controlee = 1,\
    .s.fira_config. DST_ADDR[0] = 1,\
    .s.fira_config. DST_ADDR[1] = 0,\
    .s.fira_config. Vendor_ID[0] = 0,\
    .s.fira_config. Vendor_ID[1] = 0,\
    .s.fira_config. Static_STS_IV[0] = 0,\
    .s.fira_config. Static_STS_IV[0] = 0,\
    .s.fira_config. Static_STS_IV[0] = 0,\
    .s.fira_config. Static_STS_IV[0] = 0,\
    .s.fira_config. Static_STS_IV[0] = 0,\
    .s.fira_config. Static_STS_IV[0] = 0,\
};

/* holds a run-time parameters */
struct run_s
{
    int last_error;
    int mode;

    dwt_txconfig_t  txConfig;   /**< UWB configuration */

    int16_t     rngOffset_mm;   /**< Calibration: the Ranging offset */
    uint8_t     reportLevel;    /**< 0 - no output, 1-JSON */

    uint16_t    antRx_a;        /**< antenna delay values */
    uint16_t    antTx_a;        /**< antenna delay values */

    uint8_t     xtalTrim;

    fira_device_configure_t fira_config;

};

typedef struct run_s run_t;

struct ver_num_s
{
    uint32_t ver0;
    uint32_t ver1;
    uint32_t ver2;
    uint32_t ver3;
}__attribute__((__packed__));

typedef struct ver_num_s ver_num_t;


/* The structure, holding the changeable configuration of the application
 * */
struct param_block_s 
{
    dwt_config_t    dwt_config;     /**< Standard Decawave driver config */
    ver_num_t       v;              /**< App version */
    run_t           s;              /**< Run-time parameters */
};

typedef struct param_block_s param_block_t;

#ifdef __cplusplus
}
#endif

#endif /* __DEFAULT_CONFIG__H__ */

