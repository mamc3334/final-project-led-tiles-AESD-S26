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
#include <time.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdatomic.h>
#include "config_reader.h"

#define INPUT_DIR ("/dev/input") // length = 10
#define PATH_SIZE (267) //10 + 255 + 2

#define HOLD_DELAY      (FRAME_DELAY)
#define HOLD_PERIOD     (FRAME_DELAY)
#define FRAME_DELAY     (20)

#define A_KEY (0)
#define S_KEY (1)
#define D_KEY (2)
#define F_KEY (3)
#define H_KEY (4)
#define J_KEY (5)
#define K_KEY (6)
#define L_KEY (7)
#define ESC_KEY (8)
#define ENTER_KEY (9)

#define KEYS_SIZE (10)

uint16_t input_get_keys();
int input_init();
void input_reset();


#endif /* INPUT_HANDLER_H */
