/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include "pad.h"

bool XMB = false;
bool XMB_EXIT = false;

sys_mutex_t xmb_mutex;
sys_cond_t xmb_cond;

sys_mutex_t* opt_mutex;
sys_cond_t* opt_cond;
bool opt_init = false;

/**
 * PAD Thread.
 * Checks XMB status and PAD buttons.
 */
void pad_thread(void* param) {
	//Get exit.
	bool* exit = (bool*) param;

	//Mutex & Sinal ATTR.
	sys_mutex_attr_t mutex_attr;
		memset(&mutex_attr, 0, sizeof(mutex_attr));
		mutex_attr.attr_protocol = SYS_MUTEX_PROTOCOL_FIFO;
		mutex_attr.attr_recursive = SYS_MUTEX_ATTR_NOT_RECURSIVE;
		mutex_attr.attr_pshared = SYS_MUTEX_ATTR_PSHARED;
		mutex_attr.attr_adaptive = SYS_MUTEX_ATTR_NOT_ADAPTIVE;
		strcpy(mutex_attr.name, "mutex");
	sys_cond_attr_t cond_attr;
		memset(&cond_attr, 0, sizeof(cond_attr));
		cond_attr.attr_pshared = SYS_COND_ATTR_PSHARED;
		strcpy(cond_attr.name, "cond");

	//XMB Signal.
	sysMutexCreate(&xmb_mutex, &mutex_attr);
	sysCondCreate(&xmb_cond, xmb_mutex, &cond_attr);

	//Create option mutex & signals.
	opt_mutex = malloc(sizeof(sys_mutex_t) * PAD_OPTIONS);
	opt_cond = malloc(sizeof(sys_cond_t) * PAD_OPTIONS);
	int i;
	for(i = 0; i < PAD_OPTIONS; i++) {
		sysMutexCreate(&opt_mutex[i], &mutex_attr);
		sysCondCreate(&opt_cond[i], opt_mutex[i], &cond_attr);
	}
	opt_init = true;

	//Pad init.
	padInfo padinfo;
	padData paddata;
	ioPadInit(7);

	//Callback init.
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sysutil_callback, NULL);

	//Pad loop.
	int option;
	while(!*exit) {
		//XMB check.
		sysUtilCheckCallback();

		//XMB Exit?
		if(XMB_EXIT) {
			*exit = true;
			pad_signal_opt(PAD_OPTION_EXIT);
			pad_signal_all_opt();
			break;
		}
		//XMB?
		else if(!XMB) {
			//Pad check.
			ioPadGetInfo(&padinfo);
			for(i = 0, option = 0; i < MAX_PADS; i++) {
				if(padinfo.status[i]) {
					ioPadGetData(i, &paddata);
					if(paddata.BTN_TRIANGLE) {
						option = PAD_OPTION_EXIT;
					}
					else if(paddata.BTN_START) {
						option = PAD_OPTION_BEEP;
					}
				}
			}

			//Check option selected.
			if(option == PAD_OPTION_EXIT) {
				*exit = true;
				pad_signal_opt(option);
				pad_signal_all_opt();
				break;
			}
			else if(option > 0) {
				pad_signal_opt(option);
			}
		}
	}

	console_print("[Pad] Exiting...");
	usleep(500000);

	//Pad destroy.
	ioPadEnd();

	//Callback stop.
	sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);

	//Mutex & Cond destroy.
	sysMutexDestroy(xmb_mutex);
	for(i = 0; i < PAD_OPTIONS; i++) {
		sysMutexDestroy(opt_mutex[i]);
		sysCondDestroy(opt_cond[i]);
	}

	//Exit thread.
	thread_exit();
}

/**
 * Callback to detect XMB status.
 */
void sysutil_callback(u64 status, u64 param, void *usrdata) {
	switch(status) {
		case SYSUTIL_EXIT_GAME:
			XMB_EXIT = true;
			pad_signal_xmb();
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		case SYSUTIL_MENU_OPEN:
			XMB = true;
			break;
		case SYSUTIL_MENU_CLOSE:
			XMB = false;
			pad_signal_xmb();
			break;
		default:
			break;
	}
}

/**
 * Check XMB status.
 */
bool pad_xmb_status() {
	return XMB;
}

/**
 * Waits a condition.
 */
void pad_wait(sys_mutex_t mutex, sys_cond_t cond) {
	sysMutexLock(mutex, MUTEX_TIMEOUT);
	sysCondWait(cond, COND_TIMEOUT);
	sysMutexUnlock(mutex);
}
void pad_signal(sys_mutex_t mutex, sys_cond_t cond) {
	sysMutexLock(mutex, MUTEX_TIMEOUT);
	sysCondBroadcast(cond);
	sysMutexUnlock(mutex);
}

/**
 * If XMB is open waits until it's closed.
 */
int pad_wait_xmb() {
	if(XMB) {
		pad_wait(xmb_mutex, xmb_cond);

		return 0;
	}
	else {
		return -1;
	}
}
int pad_signal_xmb() {
	pad_signal(xmb_mutex, xmb_cond);

	return 0;
}

/**
 * Waits an option to be activated.
 */
int pad_wait_opt(int option) {
	if(opt_init) {
		if(option >= PAD_MIN && option <= PAD_OPTIONS) {
			pad_wait(opt_mutex[option - 1], opt_cond[option - 1]);

			return 0;
		}
		else {
			return -1;
		}
	}
	else {
		return -2;
	}
}
int pad_signal_opt(int option) {
	if(opt_init) {
		if(option >= PAD_MIN && option <= PAD_OPTIONS) {
			pad_signal(opt_mutex[option - 1], opt_cond[option - 1]);

			return 0;
		}
		else {
			return -1;
		}
	}
	else {
		return -2;
	}
}
int pad_signal_all_opt() {
	if(opt_init) {
		int i;
		for(i = 0; i < PAD_OPTIONS; i++) {
			pad_signal_opt(i + 1);
		}

		return 0;
	}
	else {
		return -1;
	}
}
