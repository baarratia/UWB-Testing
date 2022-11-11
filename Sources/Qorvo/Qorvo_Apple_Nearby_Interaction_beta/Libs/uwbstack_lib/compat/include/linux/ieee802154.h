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

#ifndef COMPAT_LINUX_IEEE802154_H
#define COMPAT_LINUX_IEEE802154_H

#define IEEE802154_FCS_LEN 2
#define IEEE802154_FC_LEN 2
#define IEEE802154_SEQ_LEN 1

#define IEEE802154_PAN_ID_BROADCAST 0xffff
#define IEEE802154_ADDR_SHORT_BROADCAST 0xffff
#define IEEE802154_ADDR_SHORT_UNSPEC 0xfffe

#define IEEE802154_EXTENDED_ADDR_LEN 8
#define IEEE802154_SHORT_ADDR_LEN 2
#define IEEE802154_PAN_ID_LEN 2

#define IEEE802154_FC_TYPE_SHIFT 0
#define IEEE802154_FC_TYPE_MASK ((1 << 3) - 1)
#define IEEE802154_FC_TYPE_DATA 1
#define IEEE802154_FC_TYPE_ACK 2
#define IEEE802154_FC_SECEN (1 << 3)
#define IEEE802154_FC_INTRA_PAN (1 << 6)
#define IEEE802154_FC_ACK_REQ_SHIFT 5
#define IEEE802154_FC_ACK_REQ (1 << IEEE802154_FC_ACK_REQ_SHIFT)
#define IEEE802154_FC_DAMODE_SHIFT 10
#define IEEE802154_FC_VERSION_SHIFT 12
#define IEEE802154_FC_SAMODE_SHIFT 14

#define IEEE802154_FC_TYPE(x) \
	(((x)&IEEE802154_FC_TYPE_MASK) >> IEEE802154_FC_TYPE_SHIFT)

#define IEEE802154_SCF_SECLEVEL_ENC_MIC64 6

#endif /* COMPAT_LINUX_IEEE802154_H */
