#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#define CONFIG_MAX_PATH 256

//DEFAULT CONFIG
#define GPIO        18
#define MATRIX_WIDTH    16
#define MATRIX_HEIGHT   16
#define BRIGHTNESS      100
#define FPS             30
#define HIT_ZONE_ROW    1
#define NUM_PLAYERS 1
#define SONG "../beatmaps/LetitBe.csv"

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

typedef struct {
    bool running;
    bool gameover;
} GameState;

extern GameState gs;

typedef struct {
    // [display]
    uint8_t gpio_pin;
    uint8_t matrix_rows;
    uint8_t matrix_cols;
    uint8_t brightness;
    int fps;
    uint8_t hit_zone_row;

    // [colors]
    Color background;
    Color hit_zone;
    LaneColor lane_colors[4];

    // [game] 
    uint32_t score_scale;
    uint8_t num_players;
    char song[CONFIG_MAX_PATH];
} GameConfig;

/**
 * @brief parse config file and update default configuration
 * 
 * @param path config file path
 * @param out game config instance
 * 
 * @return 0 on success
 */
int config_loader_load(const char *path, GameConfig *out);

/**
 * @brief Load default configs into game config
 * 
 * @param gc game config to be loaded into
 */
void config_load_default(GameConfig *gc);

/**
 * @brief print game configs to the screen
 */
void config_loader_print(const GameConfig *s);

#endif /* CONFIG_LOADER_H */