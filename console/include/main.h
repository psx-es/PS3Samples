/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#define MUTEX_TIMEOUT 0
#define COND_TIMEOUT 0

void thread_exit();
int console_print(char* string);
int console_save(char* string);

#endif /* __MAIN_H__ */
