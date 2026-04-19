#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <stdint.h>
#include "game_config.h"

#define CONFIG_MAX_PATH 256

typedef struct {
    // [display]
    uint8_t gpio_pin;
    uint8_t matrix_rows;
    uint8_t matrix_cols;
    uint8_t brightness;
    int fps;
    int hit_zone_row;

    // [colors]
    Color background;
    Color hit_zone;
    Color miss_flash;
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
 * @brief print game configs to the screen
 */
void config_loader_print(const GameConfig *s);

#endif /* CONFIG_LOADER_H */