/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include <stdbool.h>
#include <malloc.h>
#include <string.h>

#include <io/pad.h>
#include <sysutil/sysutil.h>
#include <sys/cond.h>

#define PAD_OPTION_EXIT 1

#define XMB_MUTEX_TIMEOUT 0
#define XMB_COND_TIMEOUT 0

void thread_exit();

bool pad_get_xmb_exit();
bool pad_get_xmb();
sys_mutex_t pad_get_xmb_mutex();
sys_cond_t pad_get_xmb_cond();

void sysutil_callback(u64 status, u64 param, void *usrdata);
void pad_thread(void* param);
