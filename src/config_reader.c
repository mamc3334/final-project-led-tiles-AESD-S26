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
    gc->hit_zone_color = 9;
    gc->hit_color      = 8;
    gc->lane0_color    = 0;
    gc->lane1_color    = 1;
    gc->lane2_color    = 2;
    gc->lane3_color    = 3;
    
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
            if (strcmp(key, "hit_zone_color") == 0) out->hit_zone_color = atoi(val);
            else if (strcmp(key, "hit_color") == 0) out->hit_color = atoi(val);
            else if (strcmp(key, "lane0_color") == 0) out->lane0_color = atoi(val);
            else if (strcmp(key, "lane1_color") == 0) out->lane1_color = atoi(val);
            else if (strcmp(key, "lane2_color") == 0) out->lane2_color = atoi(val);
            else if (strcmp(key, "lane3_color") == 0) out->lane3_color = atoi(val);
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

    printf("\n[colors]\n");
    printf("  hit_zone_color = %d\n",   gc->hit_zone_color);
    printf("  hit_color      = %d\n",   gc->hit_color);
    printf("  lane0_color    = %d\n",   gc->lane0_color);
    printf("  lane1_color    = %d\n",   gc->lane1_color);
    printf("  lane2_color    = %d\n",   gc->lane2_color);
    printf("  lane3_color    = %d\n",   gc->lane3_color);

    printf("\n[game]\n");
    printf("  score_scale = %d\n", gc->score_scale);
    printf("  num_players = %d\n", gc->num_players);
    printf("  song        = %s\n", gc->song);
}
