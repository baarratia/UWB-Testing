/**
 * @file    alloc.c
 *
 * @brief   OS Abstraction Layer for alloc functions
 *
 * @section LICENSE
 *
 * Copyright 2021 (c) QORVO / Decawave Ltd, Dublin, Ireland.

 * All rights reserved.
 *
 * @section DESCRIPTION
 *
 * This file belongs to FreeRTOS OS Abstraction Layer (OSAL)
 * The purpose of this file is to redefine alloc functions.
 * The reason for doing this is to avoid dual HEAP in the program.
 * the alloc functions are redefined using FreeRTOS alloc function
 * The benefit is double. It alows the writer to writer generic and 
 * portable code using standard alloc function while abstracting the OS.
 *
 * @author Decawave Applications
 */
 
 #include <stdlib.h>
 #include <string.h>

#include "FreeRTOS.h"

//Overload alloc functions to map Freertos alloc functions

void* calloc(size_t num, size_t size)
{
    void *ret = pvPortMalloc(num*size);
    if (ret == NULL)
        return NULL;
    memset(ret,0,num*size);
    return ret;
}

void* malloc(size_t size)
{
    return pvPortMalloc(size);
}

void free(void* ptr)
{
    vPortFree(ptr);
    return;
}

struct _reent;
/*
 * The standard syscall malloc/free used in sscanf/sprintf.
 * We want them to be replaced with FreeRTOS's implementation.
 *
 * This leads that the memory allocation will be managed by FreeRTOS heap4 memory.
 * */
void* _calloc_r(struct _reent *re, size_t num, size_t size)
{
    return calloc(num, size);
}

void* _malloc_r(struct _reent *re, size_t size)
{
    return pvPortMalloc(size);
}

void _free_r(struct _reent *re, void* ptr)
{
    vPortFree(ptr);
    return;
}
