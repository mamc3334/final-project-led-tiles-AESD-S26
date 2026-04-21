/*
 * config_reader.c
 *
 *  Created on: Apr 18, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Config Handler for Piano Tiles game
 */

// Claude link: https://claude.ai/share/ef401e88-62d5-4623-9af1-6cfce6f5bac3

#include "../inc/config_reader.h"

GameConfig gc = {0};

/* Strip leading and trailing whitespace in-place. Returns pointer to s. */
/**
 * @brief strip leading and trailing whitespaces.
 * @param s input line from config file
 * @return char * of trimmed line
 */
static char *trim(char *s)
{
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

/**
 * @brief parse rgb colors
 * @param s config file line
 * @param out Color object pointer to store parsed color in
 * @return 0 on success.
 */
static int parse_color(const char *s, Color *out)
{
    int r, g, b;
    if (sscanf(s, "%d,%d,%d", &r, &g, &b) != 3) return -1;
    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) return -1;
    out->r = (uint8_t)r;
    out->g = (uint8_t)g;
    out->b = (uint8_t)b;
    return 0;
}

/**
 * @brief Load default configs into game config
 * 
 * @param gc game config to be loaded into
 */
void config_load_default(GameConfig *gc)
{
    // [display]
    gc->gpio_pin     = GPIO;
    gc->matrix_rows  = MATRIX_HEIGHT;
    gc->matrix_cols  = MATRIX_WIDTH;
    gc->brightness   = BRIGHTNESS;
    gc->fps          = 30;
    gc->hit_zone_row = HIT_ZONE_ROW;
    
    // [colors]
    gc->background = (Color){0,   0,   0  };
    gc->hit_zone   = (Color){25,  25,  25 };
    gc->lane_colors[0] = (LaneColor){{255,0,0},     {255,255,255}};
    gc->lane_colors[1] = (LaneColor){{255,100,0},   {255,255,255}};
    gc->lane_colors[2] = (LaneColor){{0,80,255},    {255,255,255}};
    gc->lane_colors[3] = (LaneColor){{160,0,255},   {255,255,255}};
 
    /* [game] */
    gc->score_scale = 10;
    gc->num_players = 1;
    strncpy(gc->song, SONG, sizeof(gc->song) - 1);
    gc->song[sizeof(gc->song) - 1] = '\0';
}

/**
 * @brief parse config file and update default configuration
 * 
 * @param path config file path
 * @param out game config instance
 * 
 * @return 0 on success
 */
int config_loader_load(const char *path, GameConfig *out)
{
    FILE *f = fopen(path, "r");

    if (!f) 
    {
        fprintf(stderr, "[config] cannot open '%s': %s\n", path, strerror(errno));
        return -1;
    }

    char line[512];
    char section[64] = "";
    int  rc = 0;
    int  lineno = 0;

    while (fgets(line, sizeof(line), f)) 
    {
        lineno++;
        char *p = trim(line);
        if (*p == '\0') continue; //ignore blank/empty lines
        if (*p == '#' || *p == ';') continue; //ignore comments

        //Parse sections
        if (*p == '[') 
        {
            char *end = strchr(p, ']');
            if (!end)
            {
                fprintf(stderr, "[config] line %d: malformed section header\n", lineno);
                rc = -2;
                continue;
            }
            *end = '\0';
            strncpy(section, p + 1, sizeof(section) - 1);
            section[sizeof(section) - 1] = '\0';
            continue;
        }

        /* Key = value */
        char *eq = strchr(p, '=');
        if (!eq) 
        {
            fprintf(stderr, "[config] line %d: missing '=', skipping\n", lineno);
            rc = -2;
            continue;
        }
        *eq = '\0';
        char *key = trim(p);
        char *val = trim(eq + 1);

        // [display]
        if (strcmp(section, "display") == 0) 
        {
            if (strcmp(key, "gpio_pin") == 0) out->gpio_pin = atoi(val);
            else if (strcmp(key, "matrix_rows") == 0) out->matrix_rows = atoi(val);
            else if (strcmp(key, "matrix_cols") == 0) out->matrix_cols = atoi(val);
            else if (strcmp(key, "brightness") == 0) out->brightness = (uint8_t)atoi(val);
            else if (strcmp(key, "fps") == 0) out->fps = atoi(val);
            else if (strcmp(key, "hit_zone_row") == 0) out->hit_zone_row = atoi(val);
        }
        // [colors]
        else if (strcmp(section, "colors") == 0) 
        {
            Color c;
            if (parse_color(val, &c) != 0) {
                fprintf(stderr, "[config] line %d: bad color value '%s'\n", lineno, val);
                rc = -2;
                continue;
            }
            if (strcmp(key, "background") == 0) out->background   = c;
            else if (strcmp(key, "hit_zone") == 0) out->hit_zone     = c;
            else if (strcmp(key, "lane0_active") == 0) out->lane_colors[0].active    = c;
            else if (strcmp(key, "lane1_active") == 0) out->lane_colors[1].active    = c;
            else if (strcmp(key, "lane2_active") == 0) out->lane_colors[2].active    = c;
            else if (strcmp(key, "lane3_active") == 0) out->lane_colors[3].active    = c;
            else if (strcmp(key, "lane0_hit") == 0) out->lane_colors[0].hit_flash = c;
            else if (strcmp(key, "lane1_hit") == 0) out->lane_colors[1].hit_flash = c;
            else if (strcmp(key, "lane2_hit") == 0) out->lane_colors[2].hit_flash = c;
            else if (strcmp(key, "lane3_hit") == 0) out->lane_colors[3].hit_flash = c;
        } 
        // [game]
        else if (strcmp(section, "game") == 0) 
        {
            if (strcmp(key, "score_scale") == 0) out->score_scale = atoi(val);
            else if (strcmp(key, "num_players") == 0) out->num_players = atoi(val);
            else if (strcmp(key, "song") == 0) {
                strncpy(out->song, val, sizeof(out->song) - 1);
                out->song[sizeof(out->song) - 1] = '\0';
            }
        } else {
            fprintf(stderr, "[config] line %d: unknown section '%s'\n", lineno, section);
            rc = -2;
        }
    }

    fclose(f);
    return rc;
}

/**
 * @brief Prints the loaded game configuration
 * 
 * @param s current game settings
 */
void config_loader_print(const GameConfig *gc)
{
    printf("=== GameSettings ===\n");
    printf("[display]\n");
    printf("  gpio_pin    = %d\n",   gc->gpio_pin);
    printf("  matrix_rows = %d\n",   gc->matrix_rows);
    printf("  matrix_cols = %d\n",   gc->matrix_cols);
    printf("  brightness  = %u\n",   gc->brightness);
    printf("  fps         = %d\n",   gc->fps);
    printf("  hit_zone_row= %d\n",   gc->hit_zone_row);
    printf("[colors]\n");
    printf("  background  = %u,%u,%u\n", gc->background.r, gc->background.g, gc->background.b);
    printf("  hit_zone    = %u,%u,%u\n", gc->hit_zone.r,   gc->hit_zone.g,   gc->hit_zone.b);
    for (int i = 0; i < 4; i++) {
        printf("  lane%d active = %u,%u,%u  hit = %u,%u,%u\n", i,
               gc->lane_colors[i].active.r,
               gc->lane_colors[i].active.g,
               gc->lane_colors[i].active.b,
               gc->lane_colors[i].hit_flash.r,
               gc->lane_colors[i].hit_flash.g,
               gc->lane_colors[i].hit_flash.b);
    }
    printf("[game]\n");
    printf("  score_scale = %d\n", gc->score_scale);
    printf("  num_players = %d\n", gc->num_players);
    printf("  song        = %s\n", gc->song);
}
