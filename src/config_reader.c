#include "config_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* -----------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------- */

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
 * @brief parse config file and update default configuration
 * 
 * @param path config file path
 * @param out game config instance
 * 
 * @return 0 on success
 */
int config_loader_load(const char *path, GameSettings *out)
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
            else if (strcmp(key, "miss_flash") == 0) out->miss_flash   = c;
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
void config_loader_print(const GameSettings *s)
{
    printf("=== GameSettings ===\n");
    printf("[display]\n");
    printf("  gpio_pin    = %d\n",   s->gpio_pin);
    printf("  matrix_rows = %d\n",   s->matrix_rows);
    printf("  matrix_cols = %d\n",   s->matrix_cols);
    printf("  brightness  = %u\n",   s->brightness);
    printf("  fps         = %d\n",   s->fps);
    printf("  hit_zone_row= %d\n",   s->hit_zone_row);
    printf("[colors]\n");
    printf("  background  = %u,%u,%u\n", s->background.r, s->background.g, s->background.b);
    printf("  hit_zone    = %u,%u,%u\n", s->hit_zone.r,   s->hit_zone.g,   s->hit_zone.b);
    printf("  miss_flash  = %u,%u,%u\n", s->miss_flash.r, s->miss_flash.g, s->miss_flash.b);
    for (int i = 0; i < 4; i++) {
        printf("  lane%d active = %u,%u,%u  hit = %u,%u,%u\n", i,
               s->lane_colors[i].active.r,
               s->lane_colors[i].active.g,
               s->lane_colors[i].active.b,
               s->lane_colors[i].hit_flash.r,
               s->lane_colors[i].hit_flash.g,
               s->lane_colors[i].hit_flash.b);
    }
    printf("[game]\n");
    printf("  score_scale = %d\n",   s->score_scale);
    printf("  num_players = %d\n",   s->num_players);
    printf("  song        = %s\n", s->song);
}