#include "frame_generator.h"

frame_buffer_t frames;
// ws2811_led_t active_color[WIDTH_BLOCK];
// ws2811_led_t inactive_color[WIDTH_BLOCK];
// // Build the colored row — one solid color across the full width
// ws2811_led_t color_row[4][WIDTH];  
// // Build a blank row (black) for spacing between color rows
// ws2811_led_t blank_row[WIDTH]; 
size_t frame_count;

static const GameConfig *cfg_ref = NULL;


static inline ws2811_led_t color_to_led(Color c)
{
    return ((ws2811_led_t)c.r << 16)
         | ((ws2811_led_t)c.g <<  8)
         |  (ws2811_led_t)c.b;
}

void frame_buffer_init(frame_buffer_t *buf)
{
    buf->frames = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

void frame_buffer_free(frame_buffer_t *buf)
{
    free(buf->frames);
    buf->frames = NULL;
    buf->count = 0;
    buf->capacity = 0;
}

int frame_buffer_push(frame_buffer_t *buf, frame_t frame)
{
    if (buf->count == buf->capacity) {
        size_t new_capacity = (buf->capacity == 0) ? 128 : buf->capacity * 2;
        frame_t *new_frames = realloc(buf->frames, new_capacity * sizeof(frame_t));
        if (!new_frames) {
            return -1;
        }
        buf->frames = new_frames;
        buf->capacity = new_capacity;
    }
    buf->frames[buf->count++] = frame;
    return 0;
}

int parse_csv_frames(const char *filename, frame_buffer_t *out)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    char line[256];

    /* Read and skip header */
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        fprintf(stderr, "Error: empty file\n");
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        frame_t frame;
        int l0, l1, l2, l3;

        /* Remove trailing newline if present */
        line[strcspn(line, "\r\n")] = '\0';

        /* Skip empty lines */
        if (line[0] == '\0') {
            continue;
        }

        if (sscanf(line, "%d,%d,%d,%d", &l0, &l1, &l2, &l3) != 4) {
            fprintf(stderr, "Warning: skipping malformed line: %s\n", line);
            continue;
        }

        frame.lane[0] = (uint8_t)l0;
        frame.lane[1] = (uint8_t)l1;
        frame.lane[2] = (uint8_t)l2;
        frame.lane[3] = (uint8_t)l3;

        if (frame_buffer_push(out, frame) != 0) {
            fclose(fp);
            fprintf(stderr, "Error: out of memory while storing frames\n");
            return -1;
        }
    }

    fclose(fp);
    return 0;
}


int init_frame(GameConfig *gc)
{
    if (!gc) {
        fprintf(stderr, "[frame] init_frame: NULL config\n");
        return -1;
    }

    cfg_ref = gc;

    if (init_led_grid() != WS2811_SUCCESS) 
    {
        fprintf(stderr, "[frame] failed to init led grid\n");
        return EXIT_FAILURE;
    }

    frame_buffer_init(&frames);

    //parse beat map
    if (parse_csv_frames(gc->song, &frames) != 0) {
        fprintf(stderr, "[frame] Failed to parse beatmap CSV\n");
        return -1;
    }

    return 0;
}

/**
 * @brief reset to start of game
 */
void start_frame()
{
    frame_count = 0;
    clear_led_grid();
}

/**
 * @brief render new frame for game
 * 
 * @param user_input bitmask of user hits - used to draw bottom lane
 * 
 * @return 0 on success and not end of song. 1 on end of song. 
 */
int render_frame(uint8_t hits, uint8_t row)
{ 
    //check if song is over
    if (frame_count >= frames.count) {
        return 1;
    }

    //push top row
    ws2811_led_t lane_color;
    
    for (int lane = 0; lane < 4; lane++) {
        if (frames.frames[frame_count].lane[lane]) {
            lane_color = color_to_led(cfg_ref->lane_colors[lane].active);
            grid_insert_lane(lane_color, lane);
        } else {
            lane_color = color_to_led(cfg_ref->background);
            grid_insert_lane(lane_color, lane);
        }
    }
    
    //update hit row
    ws2811_led_t flash_row[cfg_ref->matrix_cols];
    int width_block = cfg_ref->matrix_cols / cfg_ref->num_players / 4;

    for (uint8_t lane = 0; lane < 4; lane++) {
        uint8_t lane_bit = (1 << lane);
 
        if (hits & lane_bit) {
            for (int x = 0; x < width_block; x++) 
            {
                flash_row[x] = color_to_led(cfg_ref->lane_colors[lane].hit_flash);
            }
        } 
        else {
            for (int x = 0; x < width_block; x++) {
                flash_row[x] = color_to_led(cfg_ref->hit_zone);
            }
        }
        //TODO make lane flash row configurable
        grid_set_row_lane(flash_row, lane, row);
    }

    if (render_led_grid() != 0) 
    {
        fprintf(stderr, "[frame] render_led_grid failed at frame %lu\n", frame_count);
        
        return -1;
    }

    frame_count++;

    return 0;
}


uint32_t get_frame_index()
{
    return frame_count;
}

uint8_t get_frame(size_t idx)
{
    uint8_t frame = 0;

    for (int lane = 0; lane < 4; lane++) {
        if(frames.frames[idx].lane[lane])
        {
            frame |= (1 << lane);
        }
    }

    return frame;
}

void cleanup_frame()
{
    frame_buffer_free(&frames);
}