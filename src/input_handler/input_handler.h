/*
 * input_handler.h
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Input Handler for Piano Tiles game
 */

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/ioctl.h>

#define INPUT_DIR ("/dev/input") // length = 10
#define PATH_SIZE (267) //10 + 255 + 2

#define HOLD_DELAY 400
#define HOLD_PERIOD 100

char *get_keyboard();
void set_keyhold(int fd, unsigned int delay, unsigned int period);
void key_press(unsigned short code);
void key_release(unsigned short code);
void key_hold(unsigned short code);


#endif /* INPUT_HANDLER_H */
