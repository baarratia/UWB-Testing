/*
 * Copyright (c) 2020–2021 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and
 * may be covered by patents, patent applications, and are protected by
 * trade secret and/or copyright law. Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 * 
 */

#ifndef FIRA_HELPER_H
#define FIRA_HELPER_H

#include <stdint.h>

#include <linux/skbuff.h>
#include "fira_region_params.h"

/**
 * struct session_parameters - Fira session parameters.
 */
struct session_parameters {
	/**
	 * @device_type: see enum device_type.
	 */
	uint8_t device_type;
	/**
	 * @device_role: see enum device_role.
	 */
	uint8_t device_role;
	/**
	 * @ranging_round_usage: see enum fira_ranging_round_usage.
	 */
	uint8_t ranging_round_usage;
	/**
	 * @multi_node_mode: see enum fira_multi_node_mode.
	 */
	uint8_t multi_node_mode;
	/**
	 * @destination_short_address: Address of controller.
	 */
	uint16_t destination_short_address;
	/**
	 * @initiation_time_ms: Initiation time in duration of milliseconds.
	 */
	uint32_t initiation_time_ms;
	/**
	 * @slot_duration_rstu: Duration of a slot in RSTU. (1200RSTU=1ms)
	 */
	uint32_t slot_duration_rstu;
	/**
	 * @round_duration_slots: Number of slots per ranging round.
	 */
	uint32_t round_duration_slots;
	/**
	 * @block_duration_ms: Block size in unit of 1200 RSTU (same as ms).
	 */
	uint32_t block_duration_ms;
	/**
	 * @round_hopping: Enable FiRa round hopping.
	 */
	bool round_hopping;
	/**
	 * @channel_number: Uwb channel for this session.
	 */
	uint8_t channel_number;
	/**
	 * @preamble_code_index: Uwb preamble code index.
	 * BPRF (9-24), HPRF (25-32)
	 */
	uint8_t preamble_code_index;
	/**
	 * @rframe_config: see enum fira_rframe_config.
	 */
	uint8_t rframe_config;
	/**
	 * @sfd_id: BPRF (O or 2), HPRF (1-4).
	 */
	uint8_t sfd_id;
	/**
	 * @vupper64: vUpper64 for static STS (STATIC_STS_IV | VENDOR_ID).
	 */
	u8 vupper64[FIRA_VUPPER64_SIZE];
	/**
	 * @rx_antenna_selection: Reception antenna selection bitmask,
	 * bit 0 selects first antenna pair, etc. 0 for single RX.
	 */
	uint8_t rx_antenna_selection;
	/**
	 * @rx_antenna_pair_azimuth: Index of the antenna pair used for azimuth
	 * angle measure, or 0xff if none (default)
	 */
	uint8_t rx_antenna_pair_azimuth;
	/**
	 * @rx_antenna_pair_elevation: Index of the antenna pair used for
	 * elevation angle measure, or 0xff if none (default)
	 */
	uint8_t rx_antenna_pair_elevation;
	/**
	 * @tx_antenna_selection: Tx antenna selection bitmask, bit 0 selects
	 * antenna 0, etc., switch between ranging rounds if multiple antennas.
	 */
	uint8_t tx_antenna_selection;
	/**
	 * @rx_antenna_switch: If several antenna pairs are selected, switch
	 * pairs between ranging rounds (0).
	 * Use azimuth pair for POLL and elevation pair for FINAL messages when
	 * using DS-TWR as a responder (1).
	 */
	uint8_t rx_antenna_switch;
	/**
	 * @aoa_result_req: No local AoA report (0) or -90 to +90 (1).
	 */
	uint8_t aoa_result_req;
	/**
	 * @report_tof: No Report ToF in result message (0) or report it (1).
	 */
	uint8_t report_tof;
	/**
	 * @report_aoa_azimuth: No Report AoA azimuth in result message (0)
	 * or report it (1).
	 */
	uint8_t report_aoa_azimuth;
	/**
	 * @report_aoa_elevation: No Report AoA elevation in result message (0)
	 * or report it (1).
	 */
	uint8_t report_aoa_elevation;
	/**
	 * @report_aoa_fom: No Report AoA FOM in result message (0)
	 * or report it (1).
	 */
	uint8_t report_aoa_fom;
	/**
	 * @data_vendor_oui: Vendor OUI used to send and receive data
	 * using the ranging frames.
	 */
	u32 data_vendor_oui;
};

/**
 * struct data_parameters - Data parameters.
 */
struct data_parameters {
	/**
	 * @data_payload: Data payload to send.
	 */
	u8 data_payload[FIRA_DATA_PAYLOAD_SIZE_MAX];
	/**
	 * @data_payload_len: Length of data to send.
	 */
	u16 data_payload_len;
};

/**
 * struct controlee_parameters - Controlee parameters.
 */
struct controlee_parameters {
	/**
	 * @address: Controlee short address.
	 */
	uint16_t address;
};

/**
 * struct controlees_parameters - Controlees list parameters.
 */
struct controlees_parameters {
	/**
	 * @controlees: List of controlees.
	 */
	struct controlee_parameters controlees[FIRA_CONTROLEES_MAX];
	/**
	 * @n_controlees: Number of controlees in the list.
	 */
	int n_controlees;
};

enum aoa_measurements_index {
	FIRA_HELPER_AOA_AZIMUTH,
	FIRA_HELPER_AOA_ELEVATION,
	FIRA_HELPER_AOA_MAX
};

/**
 * struct aoa_measurements - Fira Angle of Arrival measurements.
 */
struct aoa_measurements {
	/**
	 * @rx_antenna_pair: Antenna pair index.
	 */
	uint8_t rx_antenna_pair;
	/**
	 * @aoa_fom: Estimation of local AoA reliability.
	 */
	uint8_t aoa_fom;
	/**
	 * @aoa_2pi: Estimation of reception angle.
	 */
	int16_t aoa_2pi;
};

/**
 * struct ranging_measurements - Fira ranging measurements.
 */
struct ranging_measurements {
	/**
	 * @short_addr: Address of the participating device.
	 */
	uint16_t short_addr;
	/**
	 * @status: Zero if ok, 1 otherwise.
	 */
	uint8_t status;
	/**
	 * @slot_index: in case of error, slot index where the error was detected.
	 */
	uint8_t slot_index;
	/**
	 * @stopped: Ranging was stopped as requested [controller only].
	 */
	bool stopped;
	/**
	 * @nlos: True if in non-line of sight.
	 */
	bool nlos;
	/**
	 * @los: True if in line of sight.
	 */
	bool los;
	/**
	 * @distance_mm: Distance in mm.
	 */
	int32_t distance_mm;
	/**
	 * @remote_aoa_azimuth_2pi: Estimation of reception angle in the azimuth
	 * of the participating device.
	 */
	int16_t remote_aoa_azimuth_2pi;
	/**
	 * @remote_aoa_elevation_pi: Estimation of reception angle in the
	 * elevation of the participating device.
	 */
	int16_t remote_aoa_elevation_pi;
	/**
	 * @remote_aoa_azimuth_fom: Estimation of azimuth reliability of the
	 * participating device.
	 */
	uint8_t remote_aoa_azimuth_fom;
	/**
	 * @remote_aoa_elevation_fom: Estimation of elevation of the
	 * participating device.
	 */
	uint8_t remote_aoa_elevation_fom;
	/**
	 * @local_aoa_measurements: Table of estimations of local measurements.
	 * Azimuth is stored in FIRA_HELPER_AOA_AZIMUTH, elevation is stored
	 * in FIRA_HELPER_AOA_ELEVATION.
	 */
	struct aoa_measurements local_aoa_measurements[FIRA_HELPER_AOA_MAX];
	/**
	 * @sp1_data: SP1 received data payload
	 */
	u8 sp1_data[FIRA_DATA_PAYLOAD_SIZE_MAX];
	/**
	 * @sp1_data_len: Length of received data.
	 */
	u16 sp1_data_len;
	/**
	 * @payload_seq_sent: Data sequence number.
	 */
	u32 payload_seq_sent;
};

/**
 * struct ranging_results - Fira ranging results.
 */
struct ranging_results {
	/**
	 * @stopped: Session was stopped using in band signaling from the
	 * controller (1 [controlee only]) or due to maximum attempts reached
	 * with no response (2 [controller only]).
	 */
	uint8_t stopped;
	/**
	 * @session_id: Session id of the ranging result.
	 */
	uint32_t session_id;
	/**
	 * @block_index: Current block index.
	 */
	uint32_t block_index;
	/**
	 * @ranging_interval_ms: Current ranging interval in unit of ms.
	 * formula: (block size * (stride + 1))
	 */
	uint32_t ranging_interval_ms;
	/**
	 * @timestamp_ns: Timestamp in nanoseconds in the CLOCK_MONOTONIC time
	 * reference.
	 */
	uint64_t timestamp_ns;
	/**
	 * @n_measurements: Number of measurements stored in the measurements
	 * table.
	 */
	int n_measurements;
	/**
	 * @measurements: Ranging measurements information.
	 */
	struct ranging_measurements measurements[FIRA_CONTROLEES_MAX];
};

/**
 * fira_helper_set_scheduler() - Set the scheduler and the region of fira.
 *
 * NOTE: This function must be called while the UWB MAC is stopped.
 *
 * Return: 0 or error.
 */
int fira_helper_set_scheduler(void);

/**
 * fira_helper_init_session() - Initialize a fira session.
 * @session_id: Session identifier.
 *
 * This function must be called first to create and initialize the fira session.
 *
 * Return: 0 or error.
 */
int fira_helper_init_session(uint32_t session_id);

/**
 * fira_helper_start_session() - Start a fira session.
 * @session_id: Session identifier.
 * @port_id: port id to use for notification.
 *
 * This function must be called after fira session was initialized.
 *
 * Return: 0 or error.
 */
int fira_helper_start_session(uint32_t session_id, uint32_t port_id);

/**
 * fira_helper_stop_session() - Stop a fira session.
 * @session_id: Session identifier.
 *
 * This function stop the session ranging.
 *
 * Return: 0 or error.
 */
int fira_helper_stop_session(uint32_t session_id);

/**
 * fira_helper_deinit_session() - Deinitialize a fira session.
 * @session_id: Session identifier.
 *
 * This function is called to free all memory allocated by the session.
 * This function must be called when the session is stopped.
 *
 * Return: 0 or error.
 */
int fira_helper_deinit_session(uint32_t session_id);

/**
 * fira_helper_set_session_parameters() - Set session parameters.
 * @session_id: Session identifier.
 * @session_params: Session parameters.
 *
 * Return: 0 or error.
 */
int fira_helper_set_session_parameters(
	uint32_t session_id, const struct session_parameters *session_params);

/**
 * fira_helper_add_controlees() - Add controlees to a specific session.
 * @session_id: Session identifier.
 * @controlees: List of controlees to add.
 *
 * Return: 0 or error.
 */
int fira_helper_add_controlees(uint32_t session_id,
			       const struct controlees_parameters *controlees);

/**
 * fira_helper_delete_controlees() - Delete controlees from a specific session.
 * @session_id: Session identifier.
 * @controlees: List of controlees to delete.
 *
 * Return: 0 or error.
 */
int fira_helper_delete_controlees(
	uint32_t session_id, const struct controlees_parameters *controlees);

/**
 * fira_helper_parse_ranging_report() - Parse fira ranging report.
 * @msg: The report message.
 * @results: The ranging results structure.
 *
 * Parse the fira report and populate the results structure.
 *
 * Return: 0 or error.
 */
int fira_helper_parse_ranging_report(struct sk_buff *msg,
				     struct ranging_results *results);

/**
 * fira_helper_send_data() - Set session parameters.
 * @session_id: Session identifier.
 * @data_params: Custom data parameters.
 *
 * Return: 0 or error.
 */
int fira_helper_send_data(uint32_t session_id,
			  const struct data_parameters *data_params);

#endif /* FIRA_HELPER_H */
