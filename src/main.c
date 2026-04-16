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

static uint32_t p1_score = 100;
static uint32_t p2_score = 100;

static void signal_handler(int sig);
static void parseargs(int argc, char **argv);
static void check_for_hits(uint8_t* keys, uint8_t* hits);


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
static void check_for_hits(uint8_t* keys, uint8_t* hits)
{
    uint32_t idx = get_frame_index();

    if(idx < 15) return; //not enough frames have passed to reach hit zone
    uint8_t active_lanes = get_frame(idx-15);

    for(int lane = 0; lane < 4; lane++)
    {
        if(keys[lane])
        {
            if(active_lanes & (1<<lane)) //hit
            {
                hits[lane] = 1;
                p1_score+=10;
            }
            else //miss
            {
                hits[lane] = 2;
                p1_score-=10;
            }
        }
        else if(active_lanes & (1<<lane)) //missed hit
        {
            p1_score-=10;
        }
        if(players == 2)
        {
            if(keys[lane + 4]) //player 2 keys
            {
                if(active_lanes & (1<<lane)) //hit
                {
                    hits[lane + 4] = 1;
                    p2_score+=10;
                }
                else //miss
                {
                    hits[lane + 4] = 2;
                    p2_score-=10;
                }
            }
            else if(active_lanes & (1<<lane)) //missed hit
            {
                p2_score-=10;
            }
        }
    }

    return;
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

    static uint8_t hits[FRAME_SIZE * NUM_PLAYERS] = {0};
    static uint8_t keys[KEYS_SIZE] = {0};

    while(running)
    {
        //reset frame to beginning of beat map and clear matrix
        start_frame();

        //reset input state
        input_reset();

        //get start key
        while(1)
        {
            input_get_keys(keys);
            if(keys[ENTER_KEY])
            {
                printf("Starting game...\n");
                break;
            }
            if(keys[ESC_KEY]) // handle exit
            {
                printf("Exiting game...\n");
                running = 0;
                break;
            }
        }

        while(!game_over)
        {
            //poll input
            input_get_keys(keys);
            if(keys[ESC_KEY])
            {
                printf("Exiting game...\n");
                running = 0;
                break;
            }
            while(keys[ENTER_KEY])
            {
                printf("Pausing game...\n");
                //pause game until enter key is released
                input_get_keys(keys);

                //TODO: handle frame index during pause
            }

            //check for hits and update score
            check_for_hits(keys, hits);

//if player 1 score is 0 or less, game over
//dont end in two player mode until game over
            if((players == 1) && (p1_score <= 0))
            {
                game_over = 1;
            }

            //update led matrix with hits
            //only update top row and hit zone row
            //hits: 0 for no key press, 1 for key press and hit, 2 for key press and miss
            new_frame(hits);
        }

        //game completed successfully
        if(game_over && running)
        {
            //Display score
            printf("\nGame Over!\n");
            printf("Player 1 Score: %d\n", p1_score);
#if NUM_PLAYERS == 2
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
#endif
        }
    }

    //CLEANUP
    input_cleanup();

    printf("RETURNING %d", retval);

    return retval;
}
