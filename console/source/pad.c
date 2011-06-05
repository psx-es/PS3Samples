/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include "pad.h"

int option;
sys_mutex_t option_mutex;
sys_cond_t beep_largeline_cond;

bool XMB = false;
bool XMB_EXIT = false;

sys_mutex_t xmb_mutex;
sys_cond_t xmb_cond;

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
	sysMutexCreate(&option_mutex, &mutex_attr);
	sysCondCreate(&xmb_cond, xmb_mutex, &cond_attr);
	sysCondCreate(&beep_largeline_cond, option_mutex, &cond_attr);

	//Pad init.
	padInfo padinfo;
	padData paddata;
	ioPadInit(7);

	//Callback init.
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sysutil_callback, NULL);

	//Pad loop.
	while(!*exit) {
		//XMB check.
		sysUtilCheckCallback();

		//XMB Exit?
		if(XMB_EXIT) {
			*exit = true;
			pad_signal_all_opt();
			break;
		}
		//XMB?
		else if(!XMB) {
			//Pad check.
			ioPadGetInfo(&padinfo);

			//Lock option selected.
			sysMutexLock(option_mutex, MUTEX_TIMEOUT);

			int i;
			for(i = 0, option = 0; i < MAX_PADS; i++) {
				if(padinfo.status[i]) {
					ioPadGetData(i, &paddata);
					if(paddata.BTN_TRIANGLE) {
						option = PAD_OPTION_EXIT;
					}
					else if(paddata.BTN_START) {
						option = PAD_OPTION_BEEP;
					}
					else if(paddata.BTN_SELECT) {
						option = PAD_OPTION_LARGELINE;
					}
				}
			}

			//Unlock option selected.
			sysMutexUnlock(option_mutex);

			//Check option selected.
			if(option == PAD_OPTION_EXIT) {
				*exit = true;

				pad_signal_all_opt();

				break;
			}
			else if(PAD_OPTION_BEEP || PAD_OPTION_LARGELINE) {
				pad_signal_beep_largeline();
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
	sysMutexDestroy(option_mutex);
	sysCondDestroy(beep_largeline_cond);

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
 * Custom signal.
 */
int pad_wait_beep_largeline() {
	sysMutexLock(option_mutex, MUTEX_TIMEOUT);
	sysCondWait(beep_largeline_cond, COND_TIMEOUT);

	int ret = option;

	sysMutexUnlock(option_mutex);

	return ret;
}
int pad_signal_beep_largeline() {
	pad_signal(option_mutex, beep_largeline_cond);

	return 0;
}

/**
 * Signals all options.
 */
int pad_signal_all_opt() {
	//Unblock XMB waiting.
	pad_signal_xmb();

	//Unblock other waitings.
	pad_signal_beep_largeline();

	return 0;
}
