/*
 * main.c — Example: play an MP3 in the background while doing other work
 *
 * Usage: ./mp3_player_demo <file.mp3>
 */

#include "mp3_player.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.mp3>\n", argv[0]);
        return 1;
    }

    mp3_player_t *player = mp3_player_create();
    if (!player) {
        fprintf(stderr, "Failed to create player\n");
        return 1;
    }

    printf("Starting background playback of: %s\n", argv[1]);

    if (mp3_player_play(player, argv[1]) != 0) {
        fprintf(stderr, "Failed to start playback\n");
        mp3_player_destroy(player);
        return 1;
    }

    /* --- Your main program work goes here --- */
    printf("Main thread is free to do other work while music plays...\n");

    int seconds = 0;
    while (mp3_player_is_playing(player)) {
        printf("  [%3ds] doing work...\n", ++seconds);
        sleep(1);
    }

    /* Or stop early: */
    /* sleep(5); */
    /* mp3_player_stop(player); */

    printf("Done.\n");
    mp3_player_destroy(player);
    return 0;
}
