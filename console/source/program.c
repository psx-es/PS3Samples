/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include "program.h"

void program_thread(void* param) {
	//Get exit.
	bool* exit = (bool*) param;

	//Loop.
	int beep = 0;
	char string[128];
	while(!*exit) {
		//Wait if XMB is open.
		pad_wait_xmb();

		//Beep?.
		if(pad_wait_opt(PAD_OPTION_BEEP) < 0 || *exit) {
			continue;
		}
		beep++;

		sprintf(string, "Beep [%d]", beep);
		console_print(string);
		buzzer();

		//Sleep to avoid beeping twice.
		usleep(PROGRAM_SLEEP);
	}

	console_print("[Program] Exiting...");
	usleep(500000);

	//Exit thread.
	thread_exit();
}
