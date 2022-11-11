/**
 *  @file     int_priority.h
 *
 *  @brief    Task and interrupt definitions
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#include "cmsis_os.h"

enum Priorities {
    /* Interrupts, which cannot use FreeRTOS API functions */
    PRIO_SPI1_IRQn          = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 2,
    PRIO_DMA2_Stream2_IRQn  = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 1 ,
    PRIO_DMA2_Stream3_IRQn  = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 1,

    /* Interrupt safe FreeRTOS API functions below is CMSIS IRQ's */
    PRIO_RTC_WKUP_IRQn      = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, //equivalent to the highest in the FreeRTOS
    PRIO_OTG_FS_IRQn        = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_USART3_IRQn        = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_DMA1_Stream1_IRQn  = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_DMA1_Stream3_IRQn  = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_ETH_IRQn           = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_TIM1_CC_IRQn       = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1,
    PRIO_TIM3_IRQn          = configLIBRARY_LOWEST_INTERRUPT_PRIORITY,

    /* Application-specific priorities : CMSIS-OS priorities +3..0..-3
     * osPriorityRealtime  is not used by Application level
     * osPriorityHigh       => configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + ?
     * osPriorityAboveNormal
     *
     * */
    PRIO_FlushTask          = osPriorityAboveNormal, /* FlushTask should have higher priority than CalckTask */
    PRIO_CtrlTask           = osPriorityNormal,
    PRIO_StartDefaultTask   = osPriorityLow,

    PRIO_RxTask             = osPriorityHigh,
    PRIO_CalcTask           = osPriorityNormal,
    PRIO_TrilatTask         = osPriorityBelowNormal,

    PRIO_TagPollTask        = osPriorityHigh,
    PRIO_TagRxTask          = osPriorityHigh,
    PRIO_BlinkTask          = osPriorityNormal,

    PRIO_TcfmTask           = osPriorityNormal,
    PRIO_TcwmTask           = osPriorityNormal,
    PRIO_Usb2SpiTask        = osPriorityNormal

};
