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
#define PAD_OPTIONS 2
#define PAD_MIN 1

#define XMB_MUTEX_TIMEOUT 0
#define XMB_COND_TIMEOUT 0

#define OPT_MUTEX_TIMEOUT 0
#define OPT_COND_TIMEOUT 0

void pad_thread(void* param);
void sysutil_callback(u64 status, u64 param, void *usrdata);

bool pad_xmb_status();

int pad_wait_xmb();
int pad_signal_xmb();

int pad_wait_opt(int option);
int pad_signal_opt(int option);
int pad_signal_all_opt();

#endif /* __PAD_H__ */
