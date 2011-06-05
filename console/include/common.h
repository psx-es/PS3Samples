/*
 * Copyright (C) �lvaro Fern�ndez Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>

#include <cairo/cairo.h>
#include <sys/cond.h>
#include <sys/mutex.h>
#include <sys/thread.h>

#include "rsxutil.h"
#include "pad.h"
#include "program.h"
#include "buzzer.h"

#define MAX_BUFFERS 2

#define TIMEOUT_COND 0
#define TIMEOUT_MUTEX 0

typedef struct {
	int screen_color;
	int font_color;
	double width_min;
	double height_min;
	double width_max;
	double height_max;
	double font_size;
	double font_size_div;
	double font_margin;
	double font_margin_div;
	char* font;
} console_config;

void draw_console(rsxBuffer *buffer, console_config config, int frame);

#endif /* __COMMON_H__ */
