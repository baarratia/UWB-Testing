/**
 *  @file     common_fira.c
 *
 *  @brief    Fira params control
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include "common_fira.h"
#include "deca_dbg.h"
#include "deca_interface.h"
#include "default_config.h"

static fira_param_t fira_param_;

void show_fira_params(void)
{
#define REMAINING (2048 - strlen(str))
    /* Display the Fira session */
    char *str = malloc(2048);
    int hlen;

    if (!str) {
        char *err = "not enough memory";
        port_tx_msg(err, strlen(err));
        return;
    }
    fira_param_t *fira_param = get_fira_config();
    hlen = sprintf(str, "JS%04X",
               0x5A5A); // reserve space for length of JS object
    snprintf(&str[strlen(str)], REMAINING, "{\"F PARAMS\":{\r\n");

    snprintf(&str[strlen(str)], REMAINING, "\"SLOT, rstu\":%d,\r\n",
         fira_param->session.slot_duration_rstu);
    snprintf(&str[strlen(str)], REMAINING, "\"Ranging Period, ms\":%d,\r\n",
         fira_param->session.block_duration_ms);
    snprintf(&str[strlen(str)], REMAINING,
         "\"Ranging round, slots\":%d,\r\n",
         fira_param->session.round_duration_slots);
    snprintf(&str[strlen(str)], REMAINING, "\"Session_ID\":%d,\r\n",
         (unsigned int)fira_param->session_id);
    snprintf(&str[strlen(str)], REMAINING, "\"RFRAME\":%d,\r\n",
         fira_param->session.rframe_config);
    snprintf(&str[strlen(str)], REMAINING, "\"SFD ID\":%d,\r\n",
         fira_param->session.sfd_id);
    snprintf(&str[strlen(str)], REMAINING, "\"Multi node mode\":%d,\r\n",
         fira_param->session.multi_node_mode);
    snprintf(&str[strlen(str)], REMAINING, "\"Round hopping\":%d,\r\n",
         fira_param->session.round_hopping);
    uint8_t *v = fira_param->session.vupper64;
    snprintf(&str[strlen(str)], REMAINING,
         "\"Vupper64\":%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x,\r\n",
         v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
    snprintf(&str[strlen(str)], REMAINING, "\"Initiator Addr\":0x%04X,\r\n",
         fira_param->session.destination_short_address);
    for (int i = 0; i < fira_param->controlees_params.n_controlees; i++) {
        snprintf(&str[strlen(str)], REMAINING,
             "\"Responder[%d] Addr\":0x%04X,\r\n", i,
             fira_param->controlees_params.controlees[i].address);
    }
    snprintf(
        &str[2], REMAINING, "%04X",
        strlen(str) -
            hlen); //add formatted 4X of length, this will erase first '{'
    str[hlen] = '{'; //restore the start bracket
    sprintf(&str[strlen(str)], "}\r\n");
    port_tx_msg((uint8_t *)str, strlen(str));
    free(str);

#undef REMAINING
}

void scan_fira_params(const char *text, const dwt_config_t *dwt_config, bool controller)
{
    char cmd[20];
    char vupper64[FIRA_VUPPER64_SIZE * 3];
    int param[22] = { 0 };
    int max = sizeof(param) / sizeof(*param);

    fira_param_t *fira_param = get_fira_config();
    int n = sscanf(
        text,
        "%20s %i %i %i %i %i %24s %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
        cmd, &param[0], &param[1], &param[2], &param[3], &param[4],
        vupper64, &param[5], &param[6], &param[7], &param[8], &param[9],
        &param[10], &param[11], &param[12], &param[13], &param[14],
        &param[15], &param[16], &param[17], &param[18], &param[19],
        &param[20], &param[21]);
    max = max > (n - 2) ? (n - 2) : max;

    /* TODO: Implement parsing of these parameters in the future. */
    fira_param->session.initiation_time_ms = 1000;
    fira_param->session.rx_antenna_selection = 0;
    fira_param->session.rx_antenna_pair_azimuth = 0x00;
    fira_param->session.rx_antenna_pair_elevation = 0xff;
    fira_param->session.tx_antenna_selection = 1;
    fira_param->session.rx_antenna_switch = 0;
    fira_param->session.aoa_result_req = 1;
    fira_param->session.report_tof = 1;
    fira_param->session.report_aoa_azimuth = 1;
    fira_param->session.report_aoa_fom = 1;
    /* Only supporting Deferred DS-TWR */
    fira_param->session.ranging_round_usage =
        FIRA_RANGING_ROUND_USAGE_DSTWR;

    /* Get parameters from command line. */
    int *p = param;
    /* BPRF set 1 & 2 are not allowed in Fira. */
    if (n > 1 && *p > 2) {
        /* BPRF set 3 & 5 SP1, 4 & 6 SP3. */
        fira_param->session.rframe_config =
            (*p % 2) == 0 ? FIRA_RFRAME_CONFIG_SP3 :
                    FIRA_RFRAME_CONFIG_SP1;
        /* BPRF set 3 & 4 SFD2, 5 & 6 SFD0. */
        fira_param->session.sfd_id = (*p > 4) ? FIRA_SFD_ID_0 :
                            FIRA_SFD_ID_2;
    } else {
        fira_param->session.rframe_config = FIRA_RFRAME_CONFIG_SP3;
        fira_param->session.sfd_id = FIRA_SFD_ID_2;
    }
    p++;
    fira_param->session.slot_duration_rstu = (n > 2) ? (*p++) : (2400);
    fira_param->session.block_duration_ms = (n > 3) ? (*p++) : (200);
    fira_param->session.round_duration_slots = (n > 4) ? (*p++) : (30);
    fira_param->session_id = (n > 5) ? (*p++) : (42);
    fira_param->session.multi_node_mode =
        (n > 7) ? (*p++ ? FIRA_MULTI_NODE_MODE_ONE_TO_MANY :
                  FIRA_MULTI_NODE_MODE_UNICAST) :
              (FIRA_MULTI_NODE_MODE_UNICAST);
    fira_param->session.round_hopping = (n > 8) ? (!!*p++) : (false);
    fira_param->session.destination_short_address = (n > 9) ? (*p++) : (0);
    fira_param->controlees_params.n_controlees = 0;
    for (int i = p - param; i < max; i++) {
        fira_param->controlees_params
            .controlees[fira_param->controlees_params.n_controlees++]
            .address = *p++;
    }
    /* Reusing parameters, should always be at the end of params parsing. */
    uint8_t *v = fira_param->session.vupper64;
    if (n > 6) {
        int x = sscanf(vupper64,
                   "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                   &param[0], &param[1], &param[2], &param[3],
                   &param[4], &param[5], &param[6], &param[7]);
        if (x != FIRA_VUPPER64_SIZE) {
            for (int i = 0; i < FIRA_VUPPER64_SIZE; i++)
                v[i] = i + 1;
        } else {
            for (int i = 0; i < FIRA_VUPPER64_SIZE; i++)
                v[i] = param[i];
        }
    } else {
        for (int i = 0; i < FIRA_VUPPER64_SIZE; i++)
            v[i] = i + 1;
    }

    if (fira_param->controlees_params.n_controlees == 0) {
        fira_param->controlees_params.controlees[0].address = 1;
        fira_param->controlees_params.n_controlees = 1;
    }
    /* Set controller / controlees roles. */
    if (controller) {
        fira_param->short_addr =
            fira_param->session.destination_short_address;
        fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLLER;
        fira_param->session.device_role = FIRA_DEVICE_ROLE_INITIATOR;
    } else {
        fira_param->short_addr =
            fira_param->controlees_params.controlees[0].address;
        fira_param->session.device_type = FIRA_DEVICE_TYPE_CONTROLEE;
        fira_param->session.device_role = FIRA_DEVICE_ROLE_RESPONDER;
    }
    /* Get parameters from global configuration. */
    fira_param->session.channel_number = dwt_config->chan;
    fira_param->session.preamble_code_index = dwt_config->txCode;
}

fira_param_t *get_fira_config(void)
{
    return &fira_param_;
}

