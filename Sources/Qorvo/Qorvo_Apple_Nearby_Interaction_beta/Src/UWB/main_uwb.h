/**
 * @file    main_uwb.h
 * 
 * @brief   Implementation of the Accessory UWB TWR
 *
 * @author  Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) DecaWave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef MAIN_UWB_H_
#define MAIN_UWB_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

extern void BoardInit(void);
extern int UwbInit(void);

int main_uwb(uint32_t ranging_role);
void ResumeUwbTasks(void);
void StopUwbTask(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_UWB_H_ */