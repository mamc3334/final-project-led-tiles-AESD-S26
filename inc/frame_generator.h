/*
 * fram_generator.h
 *
 *  Created on: Apr 12, 2026
 *      Author: Mason McGafifn
 *
 *  @brief Frame Generator for Piano Tiles game
 */

#ifndef FRAME_GENERATOR_H
#define FRAME_GENERATOR_H

#include "config_reader.h"

#define FRAME_SIZE (4) // 4 bytes representing the 4 lanes of the game, each byte is a bitfield representing the state of that lane for that frame (1 = active tile, 0 = no tile)

/**
 *  This is the function called by logic during initialization 
 *  to initialize the frame with the beatmap
 */
int init_frame();

/**
 * This is the function called by logic to start the frame generation process
 */
void start_frame();

void new_frame(uint8_t hits);

/**
 *  This is the function called by logic to get the current active lanes at the indicated frame index
 *  
 * This function will be used to get the active lanes for the current frame
 *   AND to update the top row of the matrix with the next frame to be rendered
 * 
 *  @param index The index of the frame to get
 *  @return A pointer to a 4 byte array representing lanes active at that frame index
 */
uint8_t get_frame(uint32_t index);

/**
 * @brief This is the function called by logic to get the current frame index using clock monontonic
 * @return The current frame index based on the time elapsed since the start of the game and the frame delay
 */
uint32_t get_frame_index();

#endif //FRAME_GENERATOR_H
