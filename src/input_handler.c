/*
 * input_handler.c
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Input Handler for Piano Tiles game
 */


#include "input_handler.h"

//restrict to game keys - uses definitions from linux/input-event-codes.h
//able to add more as needed
const char *key_names[KEY_MAX + 1] = {
    [KEY_ESC]        = "ESC",
    [KEY_A]          = "A", // For Player 2
    [KEY_S]          = "S",
    [KEY_D]          = "D",
    [KEY_F]          = "F",
    [KEY_H]          = "H", //player 1 keys
    [KEY_J]          = "J",
    [KEY_K]          = "K",
    [KEY_L]          = "L",
    [KEY_ENTER]      = "ENTER",
};

/**
 * @brief Returns path of keyboard device
 */
char *get_keyboard()
{
    DIR *dir = opendir(INPUT_DIR);
    if(!dir)
    {
        fprintf(stderr, "INPUT_HANDLER: opendir /dev/input failed. Errno: %s\r\n", strerror(errno));
        return NULL;
    }

    struct dirent *ent;
    char path[PATH_SIZE];
    char* keyboard_path = NULL;
    int fd;
    unsigned long evbits;

    while ((ent = readdir(dir)) != NULL)
    {
        //if not event skip
        if (strncmp(ent->d_name, "event", 5) != 0) continue;

        // format path
        snprintf(path, sizeof(path), "%s/%s", INPUT_DIR, ent->d_name);
        if ((fd = open(path, O_RDONLY | O_NONBLOCK)) < 0) continue;

        // Check if device has supports keyboard events
        if((ioctl(fd, EVIOCGBIT(0, sizeof(evbits)), &evbits) >= 0) && (evbits & (1 << EV_KEY)))
        {
            keyboard_path = strdup(path);
            close(fd);
            break;
        }

        close(fd);
    }

    closedir(dir);

    //FOR NOW 

    return keyboard_path;
}

void set_keyhold(int fd, unsigned int delay, unsigned int period)
{
    unsigned int repeat[2] = {delay, period};

    if (ioctl(fd, EVIOCSREP, repeat) < 0)
    {
        fprintf(stderr, "[WARNING] INPUT_HANDLER: set_keyhold failed. Errno: %s\r\n", strerror(errno));        
    }

    return;
}

void key_press(unsigned short code)
{
    printf("KEY_PRESS: %s\r\n", key_names[code]);
}

void key_release(unsigned short code)
{
    printf("KEY_RELEASE: %s\r\n", key_names[code]);
}

void key_hold(unsigned short code)
{
    printf("KEY_HOLD: %s\r\n", key_names[code]);
}