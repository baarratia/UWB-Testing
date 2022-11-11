/*! ----------------------------------------------------------------------------
 *  @file   _types.h

 *  @brief  Decawave general type definitions
 *
 * @author Decawave Applications
 * 
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 */

#ifndef _DECA_TYPES_H_
#define _DECA_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#ifndef _s64
#define _s64
typedef int64_t s64;
#endif

#ifndef _u64
#define _u64
typedef uint64_t u64;
#endif

#ifndef _s32
#define _s32
typedef int32_t s32;
#endif

#ifndef _u32
#define _u32
typedef uint32_t u32;
#endif

#ifndef _u16
#define _u16
typedef uint16_t u16;
#endif

#ifndef _s16
#define _s16
typedef int16_t s16;
#endif

#ifndef _u8
#define _u8
typedef uint8_t u8;
#endif

#ifndef _s8
#define _s8
typedef int8_t s8;
#endif

#ifndef NULL
#define NULL ((void *)0UL)
#endif

#ifdef __cplusplus
}
#endif

#endif /* DECA_TYPES_H_ */
