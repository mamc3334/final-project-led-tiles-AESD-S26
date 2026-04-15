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

int keyboard_fd = 0;
char *keyboard_path = NULL;

InputState inputState = {0};

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

void key_press(Tile *tiles, unsigned short code)
{
    KeyState key;
    unsigned short lane = 0;

    switch(code){
        case KEY_A:
            key = inputState.p2_lane1;
            lane = 1;
            break;
        
        case KEY_S:
            key = inputState.p2_lane2;
            lane = 2;
            break;
        
        case KEY_D:
            key = inputState.p2_lane3;
            lane = 3;
            break;
        
        case KEY_F:
            key = inputState.p2_lane4;
            lane = 4;
            break;
        
        case KEY_H:
            key = inputState.p1_lane1;
            lane = 1;
            break;
        
        case KEY_J:
            key = inputState.p1_lane2;
            lane = 2;
            break;
        
        case KEY_K:
            key = inputState.p1_lane3;
            lane = 3;
            break;
        
        case KEY_L:
            key = inputState.p1_lane4;
            lane = 4;
            break;
        
        case KEY_ESC:
            key = inputState.esc;
            break;
        
        case KEY_ENTER:
            key = inputState.enter;
            break;

        default:
            printf("Ignoring invalid key input: %s", key_names[code]);
            return;
    }

    if(lane>0)
    {
        calculate_score(tiles, lane, key.press_time);
    }
}

int input_init()
{
    //INITIALIZE INPUT HANDLER
    //Get keyboard input
    keyboard_path = get_keyboard();
    if(!keyboard_path)
    {
        fprintf(stderr, "INPUT_HANDLER: Could not find keyboard device. Errno: %s\r\n", strerror(errno));
        return EXIT_FAILURE;
    }
    printf("Found Keyboard: %s\r\n", keyboard_path);

    //Open keyboard
    if((keyboard_fd = open(keyboard_path, O_RDONLY)) < 0)
    {
        fprintf(stderr, "INPUT_HANDLER: Keyboard device path cannot be opened. Errno: %s\r\n", strerror(errno));
        input_cleanup();

        return EISDIR;
    }

    //Configure keyboard
    set_keyhold(keyboard_fd, HOLD_DELAY, HOLD_PERIOD);

    close(keyboard_fd);

    return 0;
}

void input_cleanup()
{
    if(keyboard_path)
    {
        free(keyboard_path);
        keyboard_path = NULL;
    }

    if(keyboard_fd >= 0) close(keyboard_fd);
}

int input_poll(Tile *tiles)
{
    if(!keyboard_path)
    {
        fprintf(stderr, "INPUT_HANDLER: Could not poll - no keyboard device. Errno: %s\r\n", strerror(errno));

        return errno;
    }

    if((keyboard_fd = open(keyboard_path, O_RDONLY)) < 0)
    {
        fprintf(stderr, "INPUT_HANDLER: Keyboard device path cannot be opened. Errno: %s\r\n", strerror(errno));

        return EISDIR;
    }

    //poll for 
    struct pollfd pfd = {
        .fd     = keyboard_fd,
        .events = POLLIN,
    };
 
    int ret = poll(&pfd, 1, FRAME_DELAY);
 
    if (ret == EINTR) return EINTR;

    if (ret < 0)
    {
        fprintf(stderr, "INPUT_HANDLER: poll failed. Errno: %s\r\n", strerror(errno));
        return -1;
    }
 
    if (ret == 0) return 0; // no input - timeout

    //INPUT RECIEVED

    struct input_event event;
    ssize_t bytes_read = read(keyboard_fd, &event, sizeof(event));
    close(keyboard_fd);
 
    if (bytes_read < 0) {
        if (errno == EINTR) return EINTR;
        fprintf(stderr, "INPUT_HANDLER: Read keyboard event. Errno: %s\r\n", strerror(errno));
        
        return EXIT_FAILURE;
    }

    //Treat oither inputs - (not key) as timeout
    if (event.type != EV_KEY)
        return 0;

    unsigned short press = 0;

    switch (event.value) {
        case PRESS: 
            key_press(tiles, event.code);
            break;
        case RELEASE: 
            key_release(tiles, event.code);
            break;
        case HOLD:
            key_hold(tiles, event.code);   
            break;

        default: 
            fprintf(stderr, "INPUT_HANDLER: Invalid key action. value=%u. code=%u\r\n", event.value, event.code); 
            break;
    }

    return 1;
}
