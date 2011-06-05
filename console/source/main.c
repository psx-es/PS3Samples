/*
 * Copyright (C) Álvaro Fernández Rojas (Noltari)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#include "common.h"

/**
 * Exit.
 */
bool GLOBAL_EXIT = false;

/**
 * Running threads counter.
 */
sys_mutex_t thread_mutex;
int THREADS_RUNNING;

/**
 * Print vars.
 * 		print_status -> 0: No print. // 1: Printing. // 2: New print.
 */
sys_mutex_t print_mutex;
sys_cond_t print_cond;
int max_lines, last_line, console_prints;
char** console_lines;
int print_status = 0;

/**
 * Main process.
 */
int main(s32 argc, const char* argv[]) {
	//Console config.
	console_config config = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Purisa"};
	config.screen_color = 0xFFFFFFFF;
	config.font_color = 0xFF000000;
	config.font_size_div = FONT_SIZE_TINY;
	config.font_margin_div = FONT_MARGIN_DIV;

	//Mutex & Sinal ATTR.
	sys_mutex_attr_t mutex_attr;
		memset(&mutex_attr, 0, sizeof(mutex_attr));
		mutex_attr.attr_protocol = SYS_MUTEX_PROTOCOL_FIFO;
		mutex_attr.attr_recursive = SYS_MUTEX_ATTR_NOT_RECURSIVE;
		mutex_attr.attr_pshared = SYS_MUTEX_ATTR_PSHARED;
		mutex_attr.attr_adaptive = SYS_MUTEX_ATTR_NOT_ADAPTIVE;
		mutex_attr.key = 0;
		mutex_attr.flags = 0;
		strcpy(mutex_attr.name, "mutex");
	sys_cond_attr_t cond_attr;
		memset(&cond_attr, 0, sizeof(cond_attr));
		cond_attr.attr_pshared = SYS_COND_ATTR_PSHARED;
		strcpy(cond_attr.name, "cond");

	//Mutex & Cond.
	sysMutexCreate(&thread_mutex, &mutex_attr);
	sysMutexCreate(&print_mutex, &mutex_attr);
	sysCondCreate(&print_cond, print_mutex, &cond_attr);
	
	//Threads.
	THREADS_RUNNING = 2;
	sys_ppu_thread_t pad_id, program_id;
	sysThreadCreate(&pad_id, pad_thread, (void*) &GLOBAL_EXIT, 1500, 0x400, 0, "pad");
	sysThreadCreate(&program_id, program_thread, (void*) &GLOBAL_EXIT, 1337, 0x400, 0, "program");

	//Create buffers.
	gcmContextData *context;
	void *host_addr = NULL;
	rsxBuffer buffers[MAX_BUFFERS];
	int currentBuffer = 0;

	//Allocate a 1Mb buffer, alligned to a 1Mb boundary to be our shared IO memory with the RSX.
	host_addr = memalign(1024*1024, HOST_SIZE);
	context = initScreen(host_addr, HOST_SIZE);

	//Get resolution.
	u16 width, height;
	getResolution(&width, &height);
	config.width_min = width * 0.06;
	config.height_min = height * 0.1;
	config.width_max = width - config.width_min;
	config.height_max = height - config.height_min;
	config.font_size = width / config.font_size_div;
	config.font_margin = config.font_size / config.font_margin_div;
	max_lines = (config.height_max - config.height_min) / (config.font_size + config.font_margin);
	max_lines++;
	console_lines = malloc(sizeof(char*) * max_lines);

	//Create buffers.
	int i;
	for(i = 0; i < MAX_BUFFERS; i++) {
		makeBuffer(&buffers[i], width, height, i);
	}
	flip(context, MAX_BUFFERS - 1);

	//Clean console lines.
	for(i = 0; i < max_lines; i++) {
		console_lines[i] = NULL;
	}

	//Main loop.
	int frames = 0;
	while(THREADS_RUNNING > 0) {
		//Prepare buffer.
		setRenderTarget(context, &buffers[currentBuffer]);
		waitFlip();

		//Frame counter.
		frames++;

		//Not XMB? -> Draw console.
		if(!pad_xmb_status() && print_status != 1) {
			if(print_status != 2) {
				sysMutexLock(print_mutex, MUTEX_TIMEOUT);
				sysCondSignal(print_cond);
				sysMutexUnlock(print_mutex);
			}

			if(print_status != 1) {
				sysMutexLock(print_mutex, MUTEX_TIMEOUT);

				//Draw into the unused buffer.
				draw_console(&buffers[currentBuffer], config, frames);

				print_status = 0;

				sysMutexUnlock(print_mutex);
			}
		}

		//Flip buffer onto screen.
		flip(context, buffers[currentBuffer].id); 

		//Change buffer.
		currentBuffer++;
		if(currentBuffer >= MAX_BUFFERS) {
			currentBuffer = 0;
		}
	}

	//Free strings.
	for(i = 0; i < max_lines; i++) {
		free(console_lines[i]);
	}
	free(console_lines);

	//Free buffers.
	gcmSetWaitFlip(context);
	for(i = 0; i < MAX_BUFFERS; i++) {
		rsxFree(buffers[i].ptr);
	}
	rsxFinish(context, 1);
	free(host_addr);

	//Mutex destroy.
	sysMutexDestroy(thread_mutex);
	sysMutexDestroy(print_mutex);

	return 0;
}

/**
 * Draws a console according to the config.
 */
void draw_console(rsxBuffer *buffer, console_config config, int frame) {
	//Cairo init.
	cairo_t *cr;
	cairo_surface_t *surface = NULL;
	surface = cairo_image_surface_create_for_data ((u8 *) buffer->ptr, CAIRO_FORMAT_RGB24, buffer->width, buffer->height, buffer->width * 4);

	if(surface != NULL) {
		//Surface create.
		cr = cairo_create(surface);

		if (cr != NULL) {
			//Clear background.
			cairo_set_source_rgb(cr, (config.screen_color >> 16) & 0xFF, (config.screen_color >> 8) & 0xFF, config.screen_color & 0xFF);
			cairo_paint(cr);

			//Font config.
			cairo_set_source_rgb(cr, (config.font_color >> 16) & 0xFF, (config.font_color >> 8) & 0xFF, config.font_color & 0xFF);
			cairo_select_font_face(cr, config.font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
			cairo_set_font_size(cr, config.font_size);

			//Text position.
			double x = config.width_min;
			double y = config.height_min;

			int i;
			if(console_lines[last_line] != NULL) {
				//Check if last line needs to be splitted.
				cairo_text_extents_t extents;
				char* line = console_lines[last_line];
				int string_length = strlen(line);

				console_lines[last_line] = NULL;

				int max = string_length;
				for(i = 0; i < max; i++) {
					char next = line[i + 1];
					line[i + 1] = '\0';

					cairo_text_extents(cr, line, &extents);

					if(i == max - 1) {
						console_save(line);
					}
					else if(extents.x_advance > config.width_max - config.width_min) {
						int buffer_length = i;
						char* buffer = malloc(sizeof(char) * buffer_length);

						int k;
						for(k = 0; k < buffer_length - 1; k++) {
							buffer[k] = line[k];
						}
						buffer[k] = '\0';
						console_save(buffer);

						line[i + 1] = next;
						int new_line_length = max - buffer_length - 1;
						char* new_line = malloc(sizeof(char) * new_line_length);
						for(k = 0; k < new_line_length - 1; k++) {
							new_line[k] = line[k + buffer_length - 1];
						}
						new_line[k] = '\0';

						free(line);
						line = new_line;

						max -= buffer_length - 1;
						i = 0;
					}
					else {
						line[i + 1] = next;
					}
				}
			}

			//Print lines.
			for(i = 0; i < max_lines; i++) {
				if(console_lines[i] != NULL) {
					cairo_move_to(cr, x, y);
					cairo_show_text(cr, console_lines[i]);
					y += config.font_size + config.font_margin;
				}
				else {
					break;
				}
			}

			//Debug info.
			#if defined(DEBUG_INFO)
				cairo_text_extents_t extents;

				cairo_set_source_rgb(cr, (DEBUG_FONT_COLOR >> 16) & 0xFF, (DEBUG_FONT_COLOR >> 8) & 0xFF, DEBUG_FONT_COLOR & 0xFF);

				char string[128];
				double debug_x = config.width_max - 20 * config.font_size;
				double debug_y = config.height_min;

				sprintf(string, "~DEBUG~");
				cairo_text_extents(cr, string, &extents);
				debug_x = config.width_max - extents.width;
				cairo_move_to(cr, debug_x, debug_y);
				cairo_show_text(cr, string);
				debug_y += config.font_size + config.font_margin;

				sprintf(string, "Frames: %d", frame);
				cairo_text_extents(cr, string, &extents);
				debug_x = config.width_max - extents.width;
				cairo_move_to(cr, debug_x, debug_y);
				cairo_show_text(cr, string);
				debug_y += config.font_size + config.font_margin;

				sprintf(string, "Prints: %d", console_prints);
				cairo_text_extents(cr, string, &extents);
				debug_x = config.width_max - extents.width;
				cairo_move_to(cr, debug_x, debug_y);
				cairo_show_text(cr, string);
				debug_y += config.font_size + config.font_margin;

				sprintf(string, "Print Status: %d", print_status);
				cairo_text_extents(cr, string, &extents);
				debug_x = config.width_max - extents.width;
				cairo_move_to(cr, debug_x, debug_y);
				cairo_show_text(cr, string);
				debug_y += config.font_size + config.font_margin;
			#endif

			//Realease Surface.
			cairo_destroy(cr);
		}

		//Flush and destroy the cairo surface.
		cairo_surface_finish(surface);
		cairo_surface_destroy(surface); 
	}
}

/**
 * Prints a string by waiting the signal of the main process.
 * Don't call it from the main process because it will wait itself.
 */
int console_print(char* string) {
	sysMutexLock(print_mutex, MUTEX_TIMEOUT);
	sysCondWait(print_cond, COND_TIMEOUT);

	print_status = 1;

	int ret = console_save(string);

	console_prints++;
	print_status = 2;
	sysMutexUnlock(print_mutex);

	return ret;
}

/**
 * Saves a string to memory.
 * Don't use if you don't know what you are doing. It can hang up the main process.
 */
int console_save(char* string) {
	if(console_lines != NULL) {
		char* new_string;
		new_string = malloc(sizeof(new_string) * strlen(string) + 1);
		strcpy(new_string, string);

		int i;
		for(i = 0; i < max_lines; i++) {
			if(console_lines[i] == NULL) {
				console_lines[i] = new_string;
				last_line = i;
				break;
			}
		}

		if(i >= max_lines) {
			for(i = 0; i < max_lines - 1; i++) {
				console_lines[i] = console_lines[i + 1];
			}

			console_lines[i] = new_string;
		}

		return 0;
	}
	else {
		return -1;
	}
}

/**
 * Exits a thread and decreases the thread counter.
 */
void thread_exit() {
	sysMutexLock(thread_mutex, MUTEX_TIMEOUT);
	THREADS_RUNNING--;
	sysMutexUnlock(thread_mutex);

	sysThreadExit(0);
}
