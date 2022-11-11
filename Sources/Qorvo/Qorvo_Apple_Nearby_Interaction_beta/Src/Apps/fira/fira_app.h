/**
 *  @file     fira_app.h
 *
 *  @brief    Fira processes control
 *
 * @author    Decawave Applications
 *
 * @attention Copyright 2017-2021 (c) Decawave Ltd, Dublin, Ireland.
 *            All rights reserved.
 *
 */

#ifndef FIRA_APP_H_
#define FIRA_APP_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

void fira_terminate(void);
void fira_helper_controller(void const *arg);
void fira_helper_controlee(void const *arg);

#ifdef __cplusplus
}
#endif

#endif /* FIRA_APP_H_ */