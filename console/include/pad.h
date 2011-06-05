/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __PAD_H__
#define __PAD_H__

#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <sys/cond.h>

#include "main.h"

#define PAD_OPTION_EXIT 1
#define PAD_OPTION_BEEP 2
#define PAD_OPTION_LARGELINE 3

void pad_thread(void* param);
void sysutil_callback(u64 status, u64 param, void *usrdata);

bool pad_xmb_status();

void pad_wait(sys_mutex_t mutex, sys_cond_t cond);
void pad_signal(sys_mutex_t mutex, sys_cond_t cond);

int pad_wait_xmb();
int pad_signal_xmb();

int pad_wait_beep_largeline();
int pad_signal_beep_largeline();

int pad_signal_all_opt();

#endif /* __PAD_H__ */
