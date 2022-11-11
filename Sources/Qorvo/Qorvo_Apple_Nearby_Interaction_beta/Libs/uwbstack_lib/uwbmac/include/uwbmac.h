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

#ifndef UWBMAC_H
#define UWBMAC_H

#include <net/netlink.h>
#include <net/genetlink.h>
#include <linux/skbuff.h>

/* Maximum of mutual report callbacks. */
#define UWBMAC_PORT_ID_MAX 10

typedef void (*frame_cb)(void *user_data, struct sk_buff *skb);

/**
 * uwbmac_init() - Initialize the UWB MAC.
 *
 * Return: 0 or error.
 */
int uwbmac_init(void);

/**
 * uwbmac_exit() - Free the UWB MAC.
 */
void uwbmac_exit(void);

/**
 * uwbmac_start() - Start the UWB MAC.
 *
 * Return: 0 or error.
 */
int uwbmac_start(void);

/**
 * uwbmac_stop() - Stop the UWB MAC.
 */
void uwbmac_stop(void);

/**
 * uwbmac_register_data_callbacks() - Set tx done and rx frame callbacks
 * @user_data: Context to give back to callback.
 * @tx_done_cb: Called when a frame has been sent.
 * @rx_frame_cb: Called when a frame is received.
 *
 * This function registers the callbacks to call in case of tx done and rx frame.
 * The callbacks are called from MAC context, big treatments should be deferred.
 *
 * NOTE: The skb sent to the callbacks should be freed by the APP using
 * the method kfree_skb.
 */
void uwbmac_register_data_callbacks(void *user_data, frame_cb tx_done_cb,
				    frame_cb rx_frame_cb);

/**
 * uwbmac_register_report_callback() - Register a report event callback for a
 * specific port.
 * @user_data: Context to give back to callback.
 * @port_id: The port id that will be used in genl_info.snd_portid of calls.
 * @cb: Callback to call when a report is available.
 *
 * This function registers the callback to call in case of a mac event.
 * The callback is called from MAC context, big treatments should be deferred.
 *
 * NOTE: The skb sent to the callback should be freed by the APP using
 * the method kfree_skb.
 *
 * Return: 0 or error.
 */
int uwbmac_register_report_callback(void *user_data, uint32_t port_id,
				    frame_cb cb);

/**
 * uwbmac_set_frame_retries() - Set number of retries.
 * @retries: Number of retries between 0 and 7.
 *
 * Set the number of tx frame retries when sending a frame with ACK.
 *
 * Return: 0 or error.
 */
int uwbmac_set_frame_retries(int retries);

/**
 * uwbmac_tx_frame() - send an uwb frame.
 * @skb: Tx buffer.
 *
 * Return: 0 or error.
 */
int uwbmac_tx_frame(struct sk_buff *skb);

/**
 * uwbmac_set_channel() - Set UWB channel to use.
 * @channel: Uwb channel, supported channels depend on driver/hardware.
 *
 * Return: 0 or error.
 */
int uwbmac_set_channel(int channel);

/**
 * uwbmac_set_pan_id() - Set pan id to use.
 * @pan_id: Pan id.
 *
 * NOTE: HW Filtering is disabled if promiscuous mode is enabled.
 *
 * Return: 0 or error.
 */
int uwbmac_set_pan_id(uint16_t pan_id);

/**
 * uwbmac_set_short_addr() - Set short address to use.
 * @short_addr: Short address.
 *
 * NOTE: HW Filtering is disabled if promiscuous mode is enabled.
 *
 * Return: 0 or error.
 */
int uwbmac_set_short_addr(uint16_t short_addr);

/**
 * uwbmac_set_extended_addr() - Set extended address to use.
 * @extended_addr: extended address.
 *
 * NOTE: HW Filtering is disabled if promiscuous mode is enabled.
 *
 * Return: 0 or error.
 */
int uwbmac_set_extended_addr(uint64_t extended_addr);

/**
 * uwbmac_set_promiscuous_mode() - Set promiscuous mode.
 * @on: True to enable promiscuous mode.
 *
 * Control hardware filtering, if promiscuous mode is enabled, the hardware
 * filtering is disabled.
 *
 * Return: 0 or error.
 */
int uwbmac_set_promiscuous_mode(bool on);

/**
 * uwbmac_set_scheduler() - Set the scheduler responsible for managing
 * the schedule, and configure its parameters.
 * @name: Scheduler name.
 * @params_attr: Scheduler paraameters.
 *
 * Device should not be started for the moment.
 *
 * Return: 0 or error.
 */
int uwbmac_set_scheduler(const char *name, const struct nlattr *params_attr);

/**
 * uwbmac_set_scheduler_parameters() - Set the scheduler parameters.
 * @name: Scheduler name.
 * @params_attr: Scheduler parameters.
 *
 * Return: 0 or error.
 */
int uwbmac_set_scheduler_parameters(const char *name,
				    const struct nlattr *params_attr);

/**
 * uwbmac_set_region_parameters() - Set region's scheduler parameters.
 * @scheduler_name: Scheduler name.
 * @region_id: Identifier of the region, scheduler specific.
 * @region_name: Name of region to attach to the scheduler.
 * @params_attr: Region parameters.
 *
 * Return: 0 or error.
 */
int uwbmac_set_region_parameters(const char *scheduler_name, uint32_t region_id,
				 const char *region_name,
				 const struct nlattr *params_attr);

/**
 * uwbmac_call_scheduler() - Call scheduler specific procedure.
 * @name: Scheduler name.
 * @call_id: Identifier of the procedure, scheduler specific.
 * @params_attr: Scheduler call parameters.
 * @info: Request information.
 *
 * Return: 0 or error.
 */
int uwbmac_call_scheduler(const char *name, uint32_t call_id,
			  const struct nlattr *params_attr,
			  const struct genl_info *info);

/**
 * uwbmac_call_region() - Call region specific procedure.
 * @scheduler_name: Scheduler name.
 * @region_id: Identifier of the region, scheduler specific.
 * @region_name: Name of the region to call.
 * @call_id: Identifier of the procedure, region specific.
 * @params_attr: Region call parameters.
 * @info: Request information.
 *
 * Return: 0 or error.
 */
int uwbmac_call_region(const char *scheduler_name, uint32_t region_id,
		       const char *region_name, uint32_t call_id,
		       const struct nlattr *params_attr,
		       const struct genl_info *info);

/**
 * uwbmac_get_version() - Get the uwbmac release version.
 *
 * Return: The release version string.
 */
const char *uwbmac_get_version(void);

#endif /* UWBMAC_H */
