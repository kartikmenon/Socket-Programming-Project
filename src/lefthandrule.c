/* ========================================================================== */
/* File: algorithm.c
 *
 * Project name: Amazing Project
 * Component name: Algorithm to find next move
 *
 * Author: Kartik Menon, Zach Kratochvil, Allison Wang
 * Date: May 2014
 *
 * Input:
 * Output:
 *
 * Pseudocode:
 * 
 */
/* ========================================================================== */

// ---------------- Open Issues

// ---------------- System includes e.g., <stdio.h>
#include <string.h>
#include <sys/socket.h>

// ---------------- Local includes  e.g., "file.h"
#include "../utils/amazing.h"
#include "../utils/header.h"
#include "../utils/AMStartup.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables

// ---------------- Private prototypes


/* ==========================================================================
 * Check if the last move wasn't a null move; basically if either the x or
 * y coordinates went somewhere
 * ========================================================================== */
int checkLastMove(myAvatar *avatar) {
    int checkX = avatar->pos.x - avatar->prev.x;
    int checkY = avatar->pos.y - avatar->prev.y;
    //printf("alg talking, dX = %d, dY = %d\n", checkX, checkY);
    if ((checkX == 0) && (checkY == 0)) {
        return 0;   // Avatar ran into a wall
    }
    
    return 1;       // Avatar went somewhere productive
}

/* ==========================================================================
 * Algorithm to determine next move for avatar; left-wall hugging
 * 
 * *** Content ***
 * If the last move succeeded (gotten from the lastMoveSuccess int in the 
 * myAvatar struct), then based on the last move (gotten from lastMoveDir int
 * in myAvatar struct) make next move. If last move worked, keep hugging left
 * wall. If not, turn around 180 degrees and use the same logic.
 * ========================================================================== */
int getMove(myAvatar *avatar) {
    
    int success = avatar->lastMoveSuccess;
    int prevMove = avatar->lastMoveDir;
    
    // Don't want to move because presumably found selected stationary avatar
    if (prevMove == M_NULL_MOVE) {
        return M_NULL_MOVE;
    }
    
    // Go left!
    if (success) {
        switch (prevMove) {
            case M_WEST:
                return M_SOUTH;
            case M_NORTH:
                return M_WEST;
            case M_SOUTH:
                return M_EAST;
            case M_EAST:
                return M_NORTH;
        }
    }
    
    // Go right!
    else {
        switch (prevMove) {
            case M_WEST:
                return M_NORTH;
            case M_NORTH:
                return M_EAST;
            case M_SOUTH:
                return M_WEST;
            case M_EAST:
                return M_SOUTH;
            
        }
    }
    return M_NULL_MOVE;  // shouldn't be called
}
