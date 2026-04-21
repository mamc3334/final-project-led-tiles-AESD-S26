#ifndef FRAME_GENERATOR_H
#define FRAME_GENERATOR_H

#include "frame_parser.h"
#include "../ecen5713_rpi_ws281x/ws2812b_wrapper.h"
#include "config_reader.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int init_frame(GameConfig *gc);
void start_frame();
uint32_t get_frame_index();
uint8_t get_frame(size_t idx);
void cleanup_frame();

int render_frame(uint8_t hits, uint8_t row);
void frame_buffer_init(frame_buffer_t *buf);
void frame_buffer_free(frame_buffer_t *buf);
int frame_buffer_push(frame_buffer_t *buf, frame_t frame);
int parse_csv_frames(const char *filename, frame_buffer_t *out);

#endif

