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
	while(!*exit) {
		//Wait if XMB is open.
		pad_wait_xmb();

		//Beep every two seconds.
		buzzer();
		sleep(2);
	}

	//Exit thread.
	thread_exit();
}
