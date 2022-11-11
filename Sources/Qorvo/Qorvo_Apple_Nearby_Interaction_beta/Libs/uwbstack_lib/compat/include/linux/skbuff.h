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

#ifndef COMPAT_LINUX_SKBUFF_H
#define COMPAT_LINUX_SKBUFF_H

#include <assert.h>
#include <linux/slab.h>
#include <linux/types.h>

struct sk_buff_head {
	struct sk_buff *next;
	struct sk_buff *prev;
};

struct sk_buff {
	struct sk_buff *next;
	struct sk_buff *prev;
	char cb[12] __attribute__((__aligned__(8)));
	unsigned int len;
	u8 *head;
	u8 *data;
	u8 *tail;
	u8 *end;
};

struct sk_buff *alloc_skb(unsigned int size, gfp_t flags);
void skb_reserve(struct sk_buff *skb, unsigned int len);
void skb_trim(struct sk_buff *skb, unsigned int len);
void *skb_put(struct sk_buff *skb, unsigned int len);
void *skb_put_data(struct sk_buff *skb, const void *data, unsigned int len);
void *skb_push(struct sk_buff *skb, unsigned int len);
void *skb_pull(struct sk_buff *skb, unsigned int len);
struct sk_buff *skb_peek(struct sk_buff_head *list);
void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk);
void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk);
struct sk_buff *skb_dequeue(struct sk_buff_head *list);
void skb_queue_purge(struct sk_buff_head *list);

static inline void kfree_skb(struct sk_buff *skb)
{
	kfree(skb);
}

static inline void skb_put_u8(struct sk_buff *skb, u8 v)
{
	u8 *p = (u8 *)skb_put(skb, 1);
	*p = v;
}

static inline bool pskb_may_pull(struct sk_buff *skb, unsigned int len)
{
	return skb->len >= len;
}

static inline void skb_queue_head_init(struct sk_buff_head *list)
{
	list->prev = list->next = (struct sk_buff *)list;
}

static inline int skb_queue_empty(const struct sk_buff_head *list)
{
	return list->next == (const struct sk_buff *)list;
}

static inline int skb_tailroom(const struct sk_buff *skb)
{
	return skb->end - skb->tail;
}

static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}

#endif /* COMPAT_LINUX_SKBUFF_H */
