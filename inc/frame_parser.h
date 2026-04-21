#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LANE_COUNT 4
typedef struct {
    uint8_t lane[LANE_COUNT];
} frame_t;

typedef struct {
    frame_t *frames;
    size_t count;
    size_t capacity;
} frame_buffer_t;
#endif
