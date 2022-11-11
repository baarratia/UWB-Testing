/**
 * @file      task_signal.h
 *
 * @brief     Header file for apps task definitions
 *
 * @author    Decawave
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef TASK_SIGNAL_H_
#define TASK_SIGNAL_H_ 1

#ifdef __cplusplus
 extern "C" {
#endif

#include "cmsis_os.h"

/* Application tasks handles & corresponded signals structure */
struct task_signal_s
{
    osThreadId Handle;     /* Task handler */
    osMutexId  MutexId;    /* Task mutex */
    int32_t    Signal;     /* Task signal */
    void       *arg;       /* additional parameters for the Task */
};

typedef struct task_signal_s    task_signal_t;

#define TERMINATE_STD_TASK(x) \
do{\
    if(x .Handle) {\
        osMutexWait(x .MutexId, osWaitForever);\
        taskENTER_CRITICAL();\
        osMutexRelease(x .MutexId);\
        if(osThreadTerminate(x .Handle) == osOK)\
        {\
            osMutexDelete(x .MutexId);\
            x .Handle = NULL;\
            x .MutexId = NULL;\
        }\
        else\
        {\
            error_handler(1, _ERR_Cannot_Delete_Task);\
        }\
        taskEXIT_CRITICAL();\
    }\
} while(0)


#ifdef __cplusplus
}
#endif

#endif /* TASK_SIGNAL_H_ */
