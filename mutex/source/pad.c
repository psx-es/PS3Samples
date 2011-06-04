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

/**
 * PAD Thread.
 * Checks XMB status and PAD buttons.
 */
void pad_thread(void* param) {
	//Get exit.
	bool* exit = (bool*) param;

	//Mutex & Sinal ATTR.
	sys_mutex_attr_t xmb_attr;
		memset(&xmb_attr, 0, sizeof(xmb_attr));
		xmb_attr.attr_protocol = SYS_MUTEX_PROTOCOL_FIFO;
		xmb_attr.attr_recursive = SYS_MUTEX_ATTR_NOT_RECURSIVE;
		xmb_attr.attr_pshared = SYS_MUTEX_ATTR_PSHARED;
		xmb_attr.attr_adaptive = SYS_MUTEX_ATTR_NOT_ADAPTIVE;
		strcpy(xmb_attr.name, "x_mutex");
	sys_cond_attr_t cond_attr;
		memset(&cond_attr, 0, sizeof(cond_attr));
		cond_attr.attr_pshared = SYS_COND_ATTR_PSHARED;
		strcpy(cond_attr.name, "x_cond");

	//XMB Signal.
	sysMutexCreate(&xmb_mutex, &xmb_attr);
	sysCondCreate(&xmb_cond, xmb_mutex, &cond_attr);

	//Pad init.
	padInfo padinfo;
	padData paddata;
	ioPadInit(7);

	//Callback init.
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, sysutil_callback, NULL);

	//Pad loop.
	int i, option;
	while(!*exit) {
		//XMB check.
		sysUtilCheckCallback();

		//XMB Exit?
		if(XMB_EXIT) {
			*exit = true;
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
				}
			}

			//Check option selected.
			if(option == PAD_OPTION_EXIT) {
				*exit = true;
			}
		}
	}

	//Pad destroy.
	ioPadEnd();

	//Callback stop.
	sysUtilUnregisterCallback(SYSUTIL_EVENT_SLOT0);

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
			sysMutexLock(xmb_mutex, XMB_MUTEX_TIMEOUT);
			sysCondBroadcast(xmb_cond);
			sysMutexUnlock(xmb_mutex);
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		case SYSUTIL_MENU_OPEN:
			XMB = true;
			break;
		case SYSUTIL_MENU_CLOSE:
			XMB = false;
			sysMutexLock(xmb_mutex, XMB_MUTEX_TIMEOUT);
			sysCondBroadcast(xmb_cond);
			sysMutexUnlock(xmb_mutex);
			break;
		default:
			break;
	}
}

/**
 * If XMB is open waits until it's closed.
 */
int pad_wait_xmb() {
	if(!XMB) {
		return -1;
	}
	else {
		sysMutexLock(xmb_mutex, XMB_MUTEX_TIMEOUT);
		sysCondWait(xmb_cond, XMB_COND_TIMEOUT);
		sysMutexUnlock(xmb_mutex);
	}
}

/**
 * XMB Status.
 */
bool pad_get_xmb() {
	return XMB;
}
bool pad_get_xmb_exit() {
	return XMB_EXIT;
}

/**
 * Mutex & Cond.
 */
sys_mutex_t pad_get_xmb_mutex() {
	return xmb_mutex;
}
sys_cond_t pad_get_xmb_cond() {
	return xmb_cond;
}