/**
 *  @file     common_fira.h
 *
 *  @brief    Header file for common_fira
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef CORE_SRC_APPS_COMMON_FIRA_H_
#define CORE_SRC_APPS_COMMON_FIRA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "deca_interface.h"
#include "fira_helper.h"
#include "niq.h"

#define BPRF_SET_1 (1) //SP0 IEEE SFD
#define BPRF_SET_2 (2) //SP0 4z SFD
#define BPRF_SET_3 (3) //SP1 4z SFD
#define BPRF_SET_4 (4) //SP3 4z SFD
#define BPRF_SET_5 (5) //SP1 IEEE SFD
#define BPRF_SET_6 (6) //SP3 IEEE SFD

//Comment below to see the debug information in a real-time, diag_printf is a buffered I/O
//#define DEBUG_SP3_MSG(...)

#ifndef DEBUG_SP3_MSG
#include "deca_dbg.h"
#define DEBUG_SP3_MSG diag_printf
#endif

#ifndef TWR_ENTER_CRITICAL
#define TWR_ENTER_CRITICAL taskENTER_CRITICAL
#endif

#ifndef TWR_EXIT_CRITICAL
#define TWR_EXIT_CRITICAL taskEXIT_CRITICAL
#endif

struct mini_diag_s { //temporary diagnostics for PDoA debugging
    uint8_t DTUNE5;
    uint32_t CIA_TDOA_0;
    uint32_t CIA_TDOA_1_PDOA;
    uint16_t IP_DIAG_10;
    uint16_t CY0_DIAG_10;
    uint16_t CY0_TOA_HI;
    uint16_t CY1_TOA_HI;
};

typedef struct mini_diag_s mini_diag_t;

struct pdoa_s {
    int16_t pdoa; /* DW3000 PDOA */
    mini_diag_t mDiag;
};

typedef struct pdoa_s pdoa_t;

struct result_s {
    uint16_t addr16;
    uint16_t rangeNum; //number from Tag Poll and Final messages, which indicates the current range number
    uint32_t resTime_us; //reception time of the end of the Final from the Tag wrt node's SuperFrame start, microseconds
    float pdoa_raw_deg; //pdoa_raw: phase differences in degrees without any correction [-180 .. 180]
    float pdoa_raw_degP; //pdoa_raw: phase differences in degrees from Poll message
    float dist_cm; //distance to the tag in cm, corrected to a rngOffset_mm
    float x_cm; //X of the tag wrt to the node, cm
    float y_cm; //Y of the tag wrt to the node, cm
    float clockOffset_pphm; //clock offset in hundredths of ppm (i.e. 1ppm is 100)
    uint16_t flag; //service message data from the tag (low byte) and node (high byte), bitmask (defined as "RES_FLAG_")
    int16_t acc_x; //Normalized accel data X from the Tag, mg: acc_x
    int16_t acc_y; //Normalized accel data Y from the Tag, mg: acc_y
    int16_t acc_z; //Normalized accel data Z from the Tag, mg: acc_z

    pdoa_t pollPDOA;
    pdoa_t finalPDOA;

    int8_t tMaster_C; //temperature of Master in degree centigrade
};

typedef struct result_s result_t;

struct fira_param_s {
    dwt_config_t dwt_config;
        uint32_t session_id;
        uint16_t short_addr;
        struct session_parameters session;
        struct controlees_parameters controlees_params;
};

typedef struct fira_param_s fira_param_t;

// ----------------------------------------------------------------------------
//
void show_fira_params();
void scan_fira_params(const char *text, const dwt_config_t *dwt_config, bool controller);
int fira_set_device_fira_from_ble(char *str, struct fira_device_configure_s * config, uint8_t aoa_chip);
fira_param_t *get_fira_config(void);
#ifdef __cplusplus
}
#endif

#endif /* CORE_SRC_APPS_COMMON_FIRA_H_ */
