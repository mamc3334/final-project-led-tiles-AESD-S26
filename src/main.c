/*
 * main.c
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Piano Tiles Game on Raspberry Pi 4 using WS2812b LED Matrix
 */

#include "input_handler.h"
#include "stdbool.h"

static volatile bool running = false;

static void signal_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        running = false;
    }
    else
    {
        fprintf(stderr, "Caught unexpected signal %d", sig);
    }
}

int main()
{
    printf("\r\nStarting Piano Tiles Game\r\n\n");

    int retval = 0;

    //Get keyboard input
    char *keyboard_path = get_keyboard();
    if(!keyboard_path)
    {
        fprintf(stderr, "INPUT_HANDLER: Could not find keyboard device. Errno: %s\r\n", strerror(errno));
        return EXIT_FAILURE;
    }
    printf("Found Keyboard: %s\r\n", keyboard_path);

    //Open keyboard
    int keyboard_fd;
    if((keyboard_fd = open(keyboard_path, O_RDONLY)) < 0)
    {
        fprintf(stderr, "INPUT_HANDLER: Keyboard device path cannot be opened. Errno: %s\r\n", strerror(errno));
        retval = EISDIR;
        goto cleanup;
    }

    //Configure keyboard
    set_keyhold(keyboard_fd, HOLD_DELAY, HOLD_PERIOD);

    //handle signals
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    struct input_event event;
    running = true;

    printf("Waiting for keypress.\r\n");

    while(running)
    {
        //TODO: add timeout so not blocking
        ssize_t bytes_read = read(keyboard_fd, &event, sizeof(event));
 
        if (bytes_read < 0) {
            if (errno == EINTR) continue; //signal - handle safely
            fprintf(stderr, "INPUT_HANDLER: Read keyboard event. Errno: %s\r\n", strerror(errno));
            retval = EXIT_FAILURE;
            goto cleanup;
        }
 
        /* We only care about key events (EV_KEY) */
        if (event.type != EV_KEY)
            continue;

        switch (event.value) {
            case 1:  key_press(event.code);   break;
            case 0:  key_release(event.code); break;
            case 2:  key_hold(event.code);    break;
            default: fprintf(stderr, "INPUT_HANDLER: Keyboard event value=%u, code=%u\r\n", event.value, event.code); break;
        }
    }

    cleanup:
    printf("CLEANUP\r\n");
    free(keyboard_path);
    keyboard_path = NULL;
    if(keyboard_fd >= 0) close(keyboard_fd);

    printf("RETURNING %d", retval);

    return retval;
}
