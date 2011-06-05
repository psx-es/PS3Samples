/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include "buzzer.h"

/**
 * LV2 Buzzer Syscall.
 */
void lv2buzzer(u64 loudness, u8 quantity, u32 type) {
	lv2syscall3(392, loudness, quantity, type);
}

/**
 * Default.
 */
void buzzer() {
	lv2buzzer(0x1004, 0xa, 0x1b6);
}
