/*
 * input_handler.c
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Input Handler for Piano Tiles game
 */


#include "input_handler.h"

static int keyboard_fd = -1;
static char *keyboard_path = NULL;
static pthread_t input_thread_id;

static atomic_uint_least16_t keyState = 0;

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

void key_event(unsigned short code, unsigned short value)
{
    int index = -1;
    switch(code){
        case KEY_A:
            index = A_KEY;
            break;
        
        case KEY_S:
            index = S_KEY;
            break;
        
        case KEY_D:
            index = D_KEY;
            break;
        
        case KEY_F:
            index = F_KEY;
            break;
        
        case KEY_H:
            index = H_KEY;
            break;
        
        case KEY_J:
            index = J_KEY;
            break;
        
        case KEY_K:
            index = K_KEY;
            break;
        
        case KEY_L:
            index = L_KEY;
            break;
        
        case KEY_ESC:
            index = ESC_KEY;
            break;
        
        case KEY_ENTER:
            index = ENTER_KEY;
            break;

        default:
            printf("Ignoring invalid key input: %u", code);
            return;
    }

    if(index >= 0)
    {
        if(value > 0)
        {
            atomic_fetch_or(&keyState, (1<<index));
        }
        else
        {
            atomic_fetch_and(&keyState, ~(1<<index));
        }
    }
    else
    {
        printf("Ignoring invalid key input: %u", code);
    }
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

// poll for input events
void *input_poll(void *arg)
{
    struct input_event event;
    ssize_t bytes_read;
    int ready;

    //open keyboard
    if((keyboard_fd = open(keyboard_path, O_RDONLY)) < 0)
    {
        fprintf(stderr, "INPUT_HANDLER: Keyboard device path cannot be opened. Errno: %s\r\n", strerror(errno));
        goto exit;
    }

    struct pollfd pfd = {
        .fd     = keyboard_fd,
        .events = POLLIN,
    };

    while(gs.running)
    {
        ready = poll(&pfd, 1, 50); //blocking poll for 50ms
 
        if (ready < 0)
        {
            if (errno == EINTR) continue; //signal
            fprintf(stderr, "INPUT_HANDLER: poll failed. Errno: %s\r\n", strerror(errno));
            goto exit;
        }
 
        if (ready == 0) continue; //timeout
 
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
        {
            fprintf(stderr, "[INPUT_HANDLER] Keyboard device error. Errno: %s\r\n", strerror(errno));
            goto exit;
        }

        bytes_read = read(keyboard_fd, &event, sizeof(event));
 
        if (bytes_read < 0) {
            if (errno == EINTR) continue; //signal - handle safely
            fprintf(stderr, "[INPUT_HANDLER] Read keyboard event. Errno: %s\r\n", strerror(errno));
            goto exit;
        }

        /* We only care about key events (EV_KEY) */
        if (event.type != EV_KEY)
            continue;

        switch (event.value) {
            case 0: case 1: case 2:  
                key_event(event.code, event.value);   
                break;
            default: 
                fprintf(stderr, "[INPUT_HANDLER] Keyboard event value=%u, code=%u\r\n", event.value, event.code); 
                break;
        }
    }

    exit:
    input_cleanup();
    pthread_exit(NULL);
}

// PUBLIC API

uint16_t input_get_keys()
{
    return atomic_load(&keyState);;
}

int input_init()
{
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

    //create thread to poll input
    int status = pthread_create(&input_thread_id, NULL, input_poll, NULL);
    if (status != 0)
    {
        fprintf(stderr, "[INPUT_HANDLER] Failed to create input thread: %s", strerror(status));
        input_cleanup();

        return EXIT_FAILURE;
    }

    status = pthread_detach(input_thread_id);
    if (status != 0)
    {
        fprintf(stderr, "[INPUT_HANDLER] Failed to detach input thread: %s", strerror(status));
        input_cleanup();

        return EXIT_FAILURE;
    }

    printf("Input handler thread initialized and detached successfully.\r\n");

    return 0;
}

void input_reset()
{
    atomic_store(&keyState, 0);
}
