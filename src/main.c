// Claude AI: https://claude.ai/chat/64445247-0e1c-4780-9e08-5e75fa7987a8
// Used by Hyounjun Chang for modifications
/*
 * main.c
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGaffin, Hyounjun Chang
 *
 *  @brief Piano Tiles Game on Raspberry Pi 4 using WS2812b LED Matrix
 */

#define _GNU_SOURCE
#include <string.h>
#include "input_handler.h"
#include "frame_generator.h"
#include "config_reader.h"
#include <stdarg.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 4096
#define OUTPUT_FILE "downloaded_beatmap"

GameState gs = {false, false};

static int32_t p1_score = 0;
static int32_t p2_score = 0;

static GameConfig gc = {0};

static void signal_handler(int sig);
static void parseargs(int argc, char **argv);
static uint8_t check_for_hits(uint8_t keys);

/*********************
 * ADITYA IMPLEMENT IN FRAME_GENERATOR- THIS IS WHAT I HAD BEFORE THO
 */

void start_frame()
{
    // frame_count = 0;
    // clear_led_grid();
    return;
}
//true = game over
bool _render_frame(uint8_t active_lanes, uint8_t row)
{
    // ...
    // BELOW IS FOR RENDERING HIT ROW
    //update hit row
    // ws2811_led_t flash_row[cfg_ref->matrix_cols];
    // int width_block = cfg_ref->matrix_cols / cfg_ref->num_players / 4;

    // for (uint8_t lane = 0; lane < 4; lane++) {
    //     uint8_t lane_bit = (1 << lane);
 
    //     if (hits & lane_bit) {
    //         for (int x = 0; x < width_block; x++) 
    //         {
    //             flash_row[x] = color_to_led(cfg_ref->lane_colors[lane].hit_flash);
    //         }
    //     } 
    //     else {
    //         for (int x = 0; x < width_block; x++) {
    //             flash_row[x] = color_to_led(cfg_ref->hit_zone);
    //         }
    //     }
    //     //TODO make lane flash row configurable
    //     grid_set_bottom_lane(flash_row, lane);
    // }

    return false;
}

static void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        gs.running = 0;
    }
    else
    {
        fprintf(stderr, "Caught unexpected signal %d", sig);
    }
}

/**
 * @brief Connects to the server and downloads the beatmap file,
 *        saving it to OUTPUT_FILE.
 * @return 0 on success, -1 on failure
 */
static int download_beatmap(void)
{
    fprintf(stdout, "Downloading beatmap from %s:%d...\n", SERVER_IP, SERVER_PORT);

    // Create socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return -1;
    }

    // Connect to server
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock_fd);
        return -1;
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Server Connection Failed");
        close(sock_fd);
        return -1;
    }

    // Send a request to trigger the file send
    const char *req = "GET\n";
    if (send(sock_fd, req, strlen(req), 0) < 0)
    {
        perror("GET request failed");
        close(sock_fd);
        return -1;
    }

    // Open output file
    int out_fd = open(OUTPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0)
    {
        perror("Could not open output file");
        close(sock_fd);
        return -1;
    }

    // Receive file data and write to disk, stop at EOF delimiter
    char buf[BUFFER_SIZE];
    const char *delim = "\n---EOF---\n";
    ssize_t n;
    size_t total = 0;

    while ((n = recv(sock_fd, buf, sizeof(buf), 0)) > 0)
    {
        // Check if this chunk contains the EOF delimiter
        char *eof_pos = memmem(buf, n, delim, strlen(delim));
        if (eof_pos)
        {
            // Write only the data before the delimiter
            ssize_t to_write = eof_pos - buf;
            if (to_write > 0)
                write(out_fd, buf, to_write);
            total += to_write;
            break;
        }

        write(out_fd, buf, n);
        total += n;
    }

    close(out_fd);
    close(sock_fd);

    fprintf(stdout, "Download complete: %zu bytes saved to " OUTPUT_FILE "\n", total);

    // Point beatmap at the downloaded file
    if(total > 0)
    {
        strncpy(gc.song, OUTPUT_FILE, sizeof(gc.song) - 1);
        gc.song[sizeof(gc.song) - 1] = '\0';
    }
    else
    {
        fprintf(stderr, "Download error: file is empty. Using default bitmap.\n");
    }

    return 0;
}

void parseargs(int argc, char **argv)
{
    int index;
    int c;
    int do_download = 0;
    int do_song = 0;

	static struct option longopts[] =
	{
		{"help", no_argument, 0, 'h'},
		{"players", required_argument, 0, 'p'},
        {"song", required_argument, 0, 's'},
        {"config", required_argument, 0, 'c'},
        {"download", no_argument,       0, 'd'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};

    while (1)
    {
        index = 0;
        c = getopt_long(argc, argv, "hp:s:dv", longopts, &index);

        if (c == -1)
            break;


        size_t len;

		switch (c)
		{
		case 0:
			/* handle flag options (array's 3rd field non-0) */
			break;

		case 'h':
			fprintf(stderr, "Usage: %s \n"
				"-h (--help)    - this information\n"
				"-p (--players) - 1 or 2 player mode (default 1)\n"
                "-s (--song)    - specify beatmap file (default 'beatmaps/LetitBe.csv'\n"
                "-c (--config)  - specify configuration file\n"
                "-d (--download) - download beatmap from server\n"
				"-v (--version) - version information\n"
				, argv[0]);
			exit(-1);

		case 'p':
            gc.num_players = atoi(optarg);

			break;

        case 's':
            do_song = 1;
            len = strlen(optarg); 
            len = (len > (CONFIG_MAX_PATH - 1)) ? (CONFIG_MAX_PATH - 1) : len;
            strncpy(gc.song, optarg, len);
            gc.song[len] = '\0';

            break;

        case 'c':
            if(config_loader_load(optarg, &gc)<0)
            {
                fprintf(stderr, "[config] error loading config file. Setting default config");
                config_load_default(&gc);
            }

            break;

        case 'd':
            do_download = 1;

            break;

        case 'v':
            //TODO: version information
            fprintf(stderr, "Piano Tiles Game on Raspberry Pi 4 using WS2812b LED Matrix\nVersion 1.0\n");
            //driver version info
            exit(-1);

        default:
            exit(-1);
        }
    }

    if (do_download && do_song)
    {
        fprintf(stderr, "Error: --download (-d) and --song (-s) are mutually exclusive.\n");
        exit(-1);
    }

    if (do_download)
    {
        fprintf(stderr, "Downloading beatmap from server.\n");
        download_beatmap();
    }
}

/**
 * @brief Checks for hits based on current keys pressed and active frame
 * @param keys Bitfield representing keys currently pressed by players (lower 4 bits for player 1, upper 4 bits for player 2)
 * 
 * @return Bitfield representing hits for each player (lower 4 bits for player 1, upper 4 bits for player 2)
 */
static uint8_t check_for_hits(uint8_t keys)
{
    uint32_t idx = get_frame_index();
    // this should not be from frame_generator.c
    // current frame should be kept by game logic after parsing files. 

    if(idx < 15) return 0; //not enough frames have passed to reach hit zone
    uint8_t active_lanes = get_frame(idx - (gc.matrix_rows - gc.hit_zone_row));
  
    printf("Frame index %d , Frame Index requested %d , active lanes %d , keys %d\r\n",idx , idx - (gc.matrix_rows - gc.hit_zone_row) , active_lanes,keys); 
    uint8_t hits = active_lanes & keys; //bitwise AND to get hits for player 1.
    	
    printf("Hits %d\r\n",hits);
    //SCORING
    uint32_t score_scale = gc.score_scale;
    p1_score += score_scale * __builtin_popcount(hits);
    p1_score -= score_scale * __builtin_popcount(active_lanes & ~keys);
    printf("score scale %d p1 score %d \r\n",score_scale , p1_score);
    uint8_t p2_keys  = keys >> 4;
    uint8_t p2_hits  = active_lanes & p2_keys;
    uint8_t p2_misses = active_lanes & ~p2_keys;

    p2_score += score_scale * __builtin_popcount(p2_hits);
    p2_score -= score_scale * __builtin_popcount(p2_misses);

    hits |= (p2_hits << 4);

    return hits;
}


int main(int argc, char **argv)
{
    printf("\r\nStarting Piano Tiles Game\r\n\n");
    
    int retval = 0;

    //handle signals
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    config_load_default(&gc);

    //parse args
    parseargs(argc, argv);

    config_loader_print(&gc);

    uint32_t frame_delay = (1000 * 1000 / gc.fps); //us

    if (init_led_grid() != WS2811_SUCCESS) {
        fprintf(stderr, "ERROR: Initializing WS2812b LED Grid failed.\n");
        return EXIT_FAILURE;
    }
    
    //load frame generator
    init_frame("../beatmaps/LetitBe.csv");

    gs.running = 1;
    
    //Get input device
    if((retval = input_init()) != 0)
    {
        fprintf(stderr, "ERROR: Initializing keyboard input failed.\n");
        return retval;
    }

    //TODO configure frame generator and led grid for custom cfg file
    
    gs.running = 1;

    uint16_t key_state = 0;

    while(gs.running)
    {
        gs.gameover = 0;
        p1_score = 100;
        p2_score = 100;

        //reset frame to beginning of beat map and clear matrix
        start_frame();

        //reset input state
        input_reset();

        //get start key
        while(1)
        {
            key_state = input_get_keys();
            if(key_state & (1 << ENTER_KEY))
            {
                printf("Starting game...\n");
                break;
            }
            if(key_state & (1 << ESC_KEY))
            {
                printf("Exiting game...\n");
                gs.running = 0;
                break;
            }
        }

        while(!gs.gameover && gs.running)
        {
            key_state = input_get_keys();
            if(key_state & (1 << ESC_KEY))
            {
                printf("Exiting game...\n");
                gs.running = 0;
                break;
            }
            while(key_state & (1 << ENTER_KEY))
            {
                printf("Pausing game...\n");
                usleep(10000); //sleep 10ms
                key_state = input_get_keys();

                //TODO: handle frame index during pause
            }

            //check for hits and update score
            uint8_t hits = check_for_hits((uint8_t)(key_state & 0xFF));

//if player 1 score is 0 or less, game over
//dont end in two player mode until game over
            if((gc.num_players == 1) && (p1_score <= 0))
            {
                p1_score = 0;
                gs.gameover = 1;
            }

            //update led matrix with hits
            //only update top row and hit zone row
            bool ret = render_frame(hits, gc.hit_zone_row);
            if(ret)
            {
            	printf("Render said frame count has passed\r\n");
                gs.gameover = 1;
                break;
            }

            usleep(frame_delay);
        }

        //game completed successfully
        if(gs.gameover && gs.running)
        {
            //Display score
            printf("\nGame Over!\n");
            printf("Player 1 Score: %d\n", p1_score);
            if(gc.num_players == 2)
            {
                printf("Player 2 Score: %d\n", p2_score);

                if(p1_score > p2_score)
                {
                    printf("Player 1 Wins!\n");
                    //matrix display winner
                }
                else if(p2_score > p1_score)
                {
                    printf("Player 2 Wins!\n");
                    //matrix display winner
                }
                else
                {
                    printf("It's a tie!\n");
                }
            }

            usleep(2000 * 1000); //sleep 2s
        }
    }

    clear_led_grid(); // turn off all led strip lights
    usleep(100 * 1000);  // 100ms per tick = ~10 rows/sec
    free_led_grid(); // free memory and buses

    printf("RETURNING %d", retval);

    return retval;
}
