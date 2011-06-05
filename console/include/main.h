/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#define FONT_MARGIN_DIV 5

#define FONT_SIZE_TINY 100
#define FONT_SIZE_SMALL 50
#define FONT_SIZE_NORMAL 33
#define FONT_SIZE_BIG 25
#define FONT_SIZE_UBERBIG 20

void thread_exit();
int console_print(char* string);
int console_save(char* string);

#endif /* __MAIN_H__ */
