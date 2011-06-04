/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include "common.h"

bool GLOBAL_EXIT = false;

sys_mutex_t thread_mutex;
int THREADS_RUNNING;

void thread_exit() {
	sysMutexLock(thread_mutex, TIMEOUT_MUTEX);
	THREADS_RUNNING--;
	sysMutexUnlock(thread_mutex);

	sysThreadExit(0);
}

int main(s32 argc, const char* argv[]) {
	//Mutex.
	sys_mutex_attr_t attr;
	memset(&attr, 0, sizeof(attr));
	attr.attr_protocol = SYS_MUTEX_PROTOCOL_FIFO;
	attr.attr_recursive = SYS_MUTEX_ATTR_NOT_RECURSIVE;
	attr.attr_pshared = SYS_MUTEX_ATTR_PSHARED;
	attr.attr_adaptive = SYS_MUTEX_ATTR_NOT_ADAPTIVE;
	strcpy(attr.name, "mutex");
	sysMutexCreate(&thread_mutex, &attr);

	//Threads.
	THREADS_RUNNING = 2;
	sys_ppu_thread_t pad_id, program_id;
	sysThreadCreate(&pad_id, pad_thread, (void*) &GLOBAL_EXIT, 1500, 0x400, 0, "pad");
	sysThreadCreate(&program_id, program_thread, (void*) &GLOBAL_EXIT, 1337, 0x400, 0, "program");

	//Create buffers.
	gcmContextData *context;
	void *host_addr = NULL;
	rsxBuffer buffers[MAX_BUFFERS];
	int currentBuffer = 0;

	//Allocate a 1Mb buffer, alligned to a 1Mb boundary to be our shared IO memory with the RSX.
	host_addr = memalign(1024*1024, HOST_SIZE);
	context = initScreen(host_addr, HOST_SIZE);

	//Get resolution and set usable screen and font size.
	u16 width, height;
	getResolution(&width, &height);

	//Create buffers.
	int i;
	for(i = 0; i < MAX_BUFFERS; i++) {
		makeBuffer(&buffers[i], width, height, i);
	}
	flip(context, MAX_BUFFERS - 1);

	//Main loop.
	while(THREADS_RUNNING > 0) {
		//Prepare buffer.
		setRenderTarget(context, &buffers[currentBuffer]);
		waitFlip();

		//Flip buffer onto screen.
		flip(context, buffers[currentBuffer].id); 

		//Change buffer.
		currentBuffer++;
		if(currentBuffer >= MAX_BUFFERS) {
			currentBuffer = 0;
		}
	}

	//Free buffers.
	gcmSetWaitFlip(context);
	for(i = 0; i < MAX_BUFFERS; i++) {
		rsxFree(buffers[i].ptr);
	}
	rsxFinish(context, 1);
	free(host_addr);

	//Mutex destroy.
	sysMutexDestroy(thread_mutex);

	return 0;
}