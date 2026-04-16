/*
 * main.c
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Piano Tiles Game on Raspberry Pi 4 using WS2812b LED Matrix
 */

#include "input_handler.h"
#include "game_config.h"
#include "frame_generator.h"
#include <stdarg.h>
#include <getopt.h>

volatile uint8_t running = 0;
static char* beatmap = SONG;
static volatile uint8_t game_over = 0;
static uint8_t players = NUM_PLAYERS;

static uint32_t p1_score = 0;
static uint32_t p2_score = 0;

static void signal_handler(int sig);
static void parseargs(int argc, char **argv);
static uint8_t check_for_hits(uint8_t keys);

static void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        running = 0;
    }
    else
    {
        fprintf(stderr, "Caught unexpected signal %d", sig);
    }
}

void parseargs(int argc, char **argv)
{
	int index;
	int c;

	static struct option longopts[] =
	{
		{"help", no_argument, 0, 'h'},
		{"players", required_argument, 0, 'p'},
        {"song", required_argument, 0, 's'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};

	while (1)
	{
		index = 0;
		c = getopt_long(argc, argv, "cd:g:his:vx:y:", longopts, &index);

		if (c == -1)
			break;

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
				"-v (--version) - version information\n"
				, argv[0]);
			exit(-1);

		case 'p':
            //MAKE GAME CONFIG 1 player
            players = atoi(optarg);

			break;

        case 's':
            //specify song
            beatmap = optarg;

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
}

/**
 * @brief Checks for hits based on current keys pressed and active frame
 * @param keys Array of currently pressed keys - 10 keys - 
 * 0-3: p1 lanes, 4-7: p2 lanes, 8: esc, 9: enter
 * 
 * @return Array of hits and misses for each lane and player
 *   - 4 or 8 values depending on number of players - 0: no hit, 1: hit
 *   - 0 for no key press, 1 for key press and hit, 2 for key press and miss
 */
static uint8_t check_for_hits(uint8_t keys)
{
    //get active lanes for hit zone row
    uint32_t idx = get_frame_index();

    if(idx < 15) return 0; //not enough frames have passed to reach hit zone
    uint8_t active_lanes = get_frame(idx-15);

    uint8_t hits = active_lanes & keys; //bitwise AND to get hits for player 1.

    hits |= (active_lanes & (keys >> 4)) << 4; //bitwise AND to get hits for player 2 and shift back to upper 4 bits

    return hits;
}

int main(int argc, char **argv)
{
    printf("\r\nStarting Piano Tiles Game\r\n\n");
    
    int retval = 0;

    //handle signals
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    //parse args
    parseargs(argc, argv);
    
    //load frame generator
    if((retval = init_frame()) != 0)
    {
        fprintf(stderr, "ERROR: Initializing frame generator failed.\n");
        return retval;
    }

    //Get input device
    if((retval = input_init()) != 0)
    {
        fprintf(stderr, "ERROR: Initializing keyboard input failed.\n");
        return retval;
    }
    
    running = 1;

    uint16_t key_state = 0;

    while(running)
    {
        game_over = 0;
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
            if(key_state & (1 << ESC_KEY)) // handle exit
            {
                printf("Exiting game...\n");
                running = 0;
                break;
            }
        }

        while(!game_over)
        {
            //poll input
            key_state = input_get_keys();
            if(key_state & (1 << ESC_KEY))
            {
                printf("Exiting game...\n");
                running = 0;
                break;
            }
            while(key_state & (1 << ENTER_KEY))
            {
                printf("Pausing game...\n");
                //pause game until enter key is released
                key_state = input_get_keys();

                //TODO: handle frame index during pause
            }

            //check for hits and update score
            uint8_t hits = check_for_hits((uint8_t)(key_state & 0xFF));

//if player 1 score is 0 or less, game over
//dont end in two player mode until game over
            if((players == 1) && (p1_score <= 0))
            {
                game_over = 1;
            }

            //update led matrix with hits
            //only update top row and hit zone row
            new_frame(hits);

            usleep(FRAME_DELAY * 1000);
        }

        //game completed successfully
        if(game_over && running)
        {
            //Display score
            printf("\nGame Over!\n");
            printf("Player 1 Score: %d\n", p1_score);
            if(players == 2)
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
        }
    }

    printf("RETURNING %d", retval);

    return retval;
}
