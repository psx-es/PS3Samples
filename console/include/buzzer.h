/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __BUZZER_H__
#define __BUZZER_H__

#include <ppu-lv2.h>

void lv2buzzer(u64 loudness, u8 quantity, u32 type);
void buzzer();

#endif /* __BUZZER_H__ */
