/*
 * main.c
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Piano Tiles Game on Raspberry Pi 4 using WS2812b LED Matrix
 */

#include "led_matrix.h"
#include "input_handler.h"
#include "game_config.h"

static volatile char running = 0;
static char* beatmap = SONG;
Tile *tiles;

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

int load_beatmap(const char* filename, Tile* tiles) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Failed to open beatmap: %s\n", filename);
        return -1;
    }

    char line[128];
    int count = 0;

    // skip header
    fgets(line, sizeof(line), f);
    Tile t;

    while (fgets(line, sizeof(line), f)) {
        if (count >= MAX_TILES) break;

        sscanf(line, "%hhu,%f,%f", &t.lane, &t.time_ms, &t.duration_ms);

        tiles[count++] = t;
    }

    fclose(f);
    return count;
}


void calculatescore()
{
    
}

void parseargs(int argc, char **argv)
{
	int index;
	int c;

	static struct option longopts[] =
	{
		{"help", no_argument, 0, 'h'},
		{"players", required_argument, 0, 'p'},
        ("song", required_argument, 0, 's'),
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
			fprintf(stderr, "%s version %s\n", argv[0], VERSION);
			fprintf(stderr, "Usage: %s \n"
				"-h (--help)    - this information\n"
				"-p (--players) - 1 or 2 player mode (default 1)\n"
                "-s (--song)    - specify beatmap file (default 'beatmaps/LetitBe.csv'\n"
				"-v (--version) - version information\n"
				, argv[0]);
			exit(-1);

		case 'p':
            //MAKE GAME CONFIG 1 player
			break;

        case 's':
            //specify song
            

            break;

		case 'v':
			fprintf(stderr, "%s version %s\n", argv[0], VERSION);
			exit(-1);

		default:
			exit(-1);
		}
	}
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
    
    //load beatmap
    tiles = (Tile *)malloc(sizeof(Tile) * MAX_TILES);
    int count = load_beatmap(beatmap, tiles);

    //Get input device
    if((retval = input_init()) != 0)
    {
        fprintf(stderr, "ERROR: Initializing keyboard input failed.\n");
        return retval;
    }
    
    //TODO: Display initial LED screen
    if((retval = matrix_init()) != 0)
    {
        fprintf(stderr, "ERROR: Initializing led matrix failed.\n");
        return retval;
    }

    running = 1;

    while(running)
    {
        printf("Press Enter to Begin the Game.\r\n");

        while(inputState.enter.value != PRESS);

        while(!game_over)
        {
            //poll input during frame
            if((retval = input_poll()) < 0) running = 0; break;

            //update frame
            if((retval = frame_render(&inputState)) < 0) running = 0; break;

            //matrix update with new row
            if((retval = matrix_update()) < 0) running = 0; break;
        }

        //game completed successfully
        if(game_over && running)
        {
            //Display score

        }
    }

    //CLEANUP
    input_cleanup();

    printf("RETURNING %d", retval);

    return retval;
}
