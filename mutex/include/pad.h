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

#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <sys/cond.h>

#include "main.h"

#define PAD_OPTION_EXIT 1
#define PAD_OPTIONS 1
#define PAD_MIN 1

#define XMB_MUTEX_TIMEOUT 0
#define XMB_COND_TIMEOUT 0

int pad_wait_xmb();
int pad_signal_xmb();

int pad_wait_opt(int option);
int pad_signal_opt(int option);

void sysutil_callback(u64 status, u64 param, void *usrdata);
void pad_thread(void* param);

#endif /* __PAD_H__ */
