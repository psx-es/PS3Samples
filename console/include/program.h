/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <unistd.h>
#include <string.h>

#include "buzzer.h"
#include "pad.h"

#define PROGRAM_SLEEP 500000

void program_thread(void* param);

#endif /* __PROGRAM_H__ */
