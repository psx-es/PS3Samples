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
	char string[1024];
	while(!*exit) {
		//Wait if XMB is open.
		pad_wait_xmb();

		//Beep?.
		int option = pad_wait_beep_largeline();
		if(*exit) {
			continue;
		}

		if(option == PAD_OPTION_BEEP) {
			beep++;

			sprintf(string, "Beep [%d]", beep);
			console_print(string);
			buzzer();

			//Sleep to avoid beeping twice.
			usleep(PROGRAM_SLEEP);
		}
		else if(option == PAD_OPTION_LARGELINE) {
			sprintf(string, "This is a very very very large line to test window fitting of printed text. Remind that PSL1GHT Console is an open source project :D. 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
			console_print(string);

			//Sleep to avoid beeping twice.
			usleep(PROGRAM_SLEEP);
		}
	}

	console_print("[Program] Exiting...");
	usleep(500000);

	//Exit thread.
	thread_exit();
}
