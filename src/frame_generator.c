/*
 * frame_generator.c
 *
 *  Created on: Apr 04, 2026
 *      Author: Mason McGafifn
 *
 *  @brief frame generator for Piano Tiles game
 */


#include "frame_generator.h"

/**
 *  This is the function called by logic during initialization 
 *  to initialize the frame with the beatmap
 */
int init_frame()
{
    return 0;
}

/**
 * This is the function called by logic to start/restart the frame generation process
 */
void start_frame()
{

}

//update top row of matrix with current frame
//update hit zone row with hits from check_for_hits
void new_frame(uint8_t hits)
{
    
}

/**
 *  This is the function called by logic to get the current frame
 *  @return A pointer to a 4 byte array representing the key that is supposed to be pressed by the user for that particular frame that is rendered
 */
uint8_t get_frame(uint32_t index)
{
    return 0;
}

/**
 * @brief This is the function called by logic to get the current frame index using clock monontonic
 * @return The current frame index based on the time elapsed since the start of the game and the frame delay
 */
uint32_t get_frame_index()
{
    return 0;
}