/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>

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

#endif /* __COMMON_H__ */
