/*
 * game_state.h
 *
 *  Created on: Apr 12, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Struct for Piano Tiles game
 */

#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdint.h>

//DEFAULT CONFIG
#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define HIT_ZONE_ROW   14

#define NUM_PLAYERS 1
#define SONG "../beatmaps/LetitBe.csv"
extern volatile uint8_t running;

// -------- COLORS --------
typedef struct {
    uint8_t r, g, b;
} Color;

static const Color COLOR_BACKGROUND = {0, 0, 0};
static const Color COLOR_HIT_ZONE   = {25, 25, 25};
static const Color COLOR_MISS       = {160, 0, 0};

typedef struct {
    Color active;
    Color hit_flash;
} LaneColor;

static const LaneColor lane_colors[4] = {
    {{0,160,255}, {255,255,255}},
    {{0,230,100}, {255,255,255}},
    {{220,60,200}, {255,255,255}},
    {{255,150,0}, {255,255,255}}
};

#endif
