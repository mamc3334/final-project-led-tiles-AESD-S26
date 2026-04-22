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
#define GPIO        10
#define MATRIX_WIDTH    16
#define MATRIX_HEIGHT   16
#define BRIGHTNESS      100
#define FPS             30
#define HIT_ZONE_ROW    1
#define NUM_PLAYERS 1
#define SONG "../beatmaps/LetitBe.csv"

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
    uint8_t hit_zone_color;
    uint8_t hit_color;
    uint8_t lane0_color;
    uint8_t lane1_color;
    uint8_t lane2_color;
    uint8_t lane3_color;

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
