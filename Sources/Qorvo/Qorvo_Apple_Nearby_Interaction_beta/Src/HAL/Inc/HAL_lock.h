/**
 * @file    HAL_lock.h
 *
 * @brief   Header for HAL_lock
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef HAL_LOCK_H
#define HAL_LOCK_H
typedef enum 
{
  DW_HAL_NODE_UNLOCKED  = 0,
  DW_HAL_NODE_LOCKED    = 1
} dw_hal_lockTypeDef;


#define __HAL_LOCK(__HANDLE__)                                           \
                                do{                                        \
                                    if((__HANDLE__)->lock == DW_HAL_NODE_LOCKED)   \
                                    {                                      \
                                       return -1;                    \
                                    }                                      \
                                    else                                   \
                                    {                                      \
                                       (__HANDLE__)->lock = DW_HAL_NODE_LOCKED;    \
                                    }                                      \
                                }while (0U)

 #define __HAL_UNLOCK(__HANDLE__)                                          \
                                  do{                                       \
                                    (__HANDLE__)->lock = DW_HAL_NODE_UNLOCKED;    \
                                  }while (0U)
#endif
