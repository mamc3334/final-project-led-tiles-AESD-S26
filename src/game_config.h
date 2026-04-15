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

#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define HIT_ZONE_ROW   14

typedef enum {
    MODE_SINGLE_PLAYER,
    MODE_TWO_PLAYER
} GameMode;

#define CURRENT_MODE MODE_SINGLE_PLAYER

typedef struct {
    uint8_t lane;
    char key;
    uint8_t col_start;
    uint8_t col_end;
} LaneConfig;

// -------- SINGLE PLAYER --------
#define SINGLE_LANE_WIDTH 2
#define SINGLE_LANE_COUNT 4

static const LaneConfig single_player_lanes[SINGLE_LANE_COUNT] = {
    {0, 'A', 4, 5},
    {1, 'S', 6, 7},
    {2, 'D', 8, 9},
    {3, 'F', 10, 11}
};

// -------- TWO PLAYER --------
#define TWO_LANE_WIDTH 1

static const uint8_t white_divider_cols[2] = {7, 8};

// Player 1
static const LaneConfig p1_lanes[4] = {
    {0, 'A', 2, 2},
    {1, 'S', 3, 3},
    {2, 'D', 4, 4},
    {3, 'F', 5, 5}
};

// Player 2
static const LaneConfig p2_lanes[4] = {
    {0, 'H', 10, 10},
    {1, 'J', 11, 11},
    {2, 'K', 12, 12},
    {3, 'L', 13, 13}
};

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

// -------- SCORING --------
#define POINTS_PERFECT 100
#define POINTS_GOOD    50
#define POINTS_MISS    0

// -------- BEATMAP --------
#define SONG "../beatmaps/LetitBe.csv"

typedef struct {
    uint8_t lane;
    float time_ms;
    float duration_ms;
} Tile;

#define MAX_TILES 2048

typedef struct KeyState{
    uint64_t press_time;
    unsigned short value;
}KeyState;

typedef struct InputState{
    KeyState p1_lane1;
    KeyState p1_lane2;
    KeyState p1_lane3;
    KeyState p1_lane4;
    KeyState p2_lane1;
    KeyState p2_lane2;
    KeyState p2_lane3;
    KeyState p2_lane4;
    KeyState esc;
    KeyState enter;
}InputState;

#endif
