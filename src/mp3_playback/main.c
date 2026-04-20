// Generated from Claude AI: https://claude.ai/chat/e05980ff-3152-4e54-8500-b6134312b4
16
// Reviewed by: Hyounjun Chang

/*
 * main.c — Example: play an MP3 in the background while doing other work
 *
 * Usage: ./mp3_player_demo <file.mp3>
 *
 * Controls (press Enter after each key):
 *   p — pause
 *   r — resume
 *   q — quit
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

    printf("Controls: [p] pause  [r] resume  [q] quit\n");

    int ch;
    while ((ch = getchar()) != EOF) {
        if (ch == '\n') continue;

        switch (ch) {
        case 'p':
            mp3_player_pause(player);
            printf("Paused at %02d:%02d\n",
                   (int)mp3_player_get_position(player) / 60,
                   (int)mp3_player_get_position(player) % 60);
            break;
        case 'r':
            mp3_player_resume(player);
            printf("Resumed\n");
            break;
        case 'q':
            mp3_player_stop(player);
            goto done;
        default:
            printf("Unknown command. Controls: [p] pause  [r] resume  [q] quit\n");
            break;
        }

        if (!mp3_player_is_playing(player)) {
            printf("Playback finished.\n");
            break;
        }
    }

done:
    mp3_player_destroy(player);
    return 0;
}
