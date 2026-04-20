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

#include "input_handler.h"
#include "game_config.h"
#include "frame_generator.h"
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
        perror("connect");
        close(sock_fd);
        return -1;
    }

    // Send a request to trigger the file send
    const char *req = "GET\n";
    if (send(sock_fd, req, strlen(req), 0) < 0)
    {
        perror("send");
        close(sock_fd);
        return -1;
    }

    // Open output file
    int out_fd = open(OUTPUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0)
    {
        perror("open output file");
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
    beatmap = OUTPUT_FILE;

    return 0;
}

void parseargs(int argc, char **argv)
{
    int index;
    int c;

    static struct option longopts[] =
    {
        {"help",     no_argument,       0, 'h'},
        {"players",  required_argument, 0, 'p'},
        {"song",     required_argument, 0, 's'},
        {"download", no_argument,       0, 'd'},
        {"version",  no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    while (1)
    {
        index = 0;
        c = getopt_long(argc, argv, "hp:s:dv", longopts, &index);

        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            break;

        case 'h':
            fprintf(stderr, "Usage: %s \n"
                "-h (--help)     - this information\n"
                "-p (--players)  - 1 or 2 player mode (default 1)\n"
                "-s (--song)     - specify beatmap file (default 'beatmaps/LetitBe.csv')\n"
                "-d (--download) - download beatmap from server (%s:%d)\n"
                "-v (--version)  - version information\n"
                , argv[0], SERVER_IP, SERVER_PORT);
            exit(-1);

        case 'p':
            //MAKE GAME CONFIG 1 player
            players = atoi(optarg);
            break;

        case 's':
            //specify song
            beatmap = optarg;
            break;

        case 'd':
            fprintf(stderr, "Downloading Beatmap from server.\n");
            download_beatmap();
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
 * @param keys Bitfield representing keys currently pressed by players (lower 4 bits for player 1, upper 4 bits for player 2)
 * 
 * @return Bitfield representing hits for each player (lower 4 bits for player 1, upper 4 bits for player 2)
 */
static uint8_t check_for_hits(uint8_t keys)
{
    uint32_t idx = get_frame_index();

    if(idx < 15) return 0; //not enough frames have passed to reach hit zone
    uint8_t active_lanes = get_frame(idx-15);

    uint8_t hits = active_lanes & keys; //bitwise AND to get hits for player 1.

    //SCORING
    p1_score += 10 * __builtin_popcount(hits);
    p1_score -= 10 * __builtin_popcount(active_lanes & ~keys);

    uint8_t p2_keys  = keys >> 4;
    uint8_t p2_hits  = active_lanes & p2_keys;
    uint8_t p2_misses = active_lanes & ~p2_keys;

    p2_score += 10 * __builtin_popcount(p2_hits);
    p2_score -= 10 * __builtin_popcount(p2_misses);

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
            if(key_state & (1 << ESC_KEY))
            {
                printf("Exiting game...\n");
                running = 0;
                break;
            }
        }

        while(!game_over)
        {
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