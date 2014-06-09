/* ========================================================================== */
/* File: maze.h
 *
 * Project name: Amazing Project
 * Component name: Essentially the graphics header file.
 *
 * Author: Kartik Menon, Zach Kratochvil, Allison Wang
 * Date: May 2014
 *
 */
/* ========================================================================== */

#ifndef MAZE_H
#define MAZE_H

#define _XOPEN_SOURCE 700

#include <stdint.h>
#include <stdbool.h>
#include "./header.h"

/*
*Initializes char *maze in shared memory of specified width and height.
*@maze -the pointer to the maze's location in shared memory (no longer in use)
*@width
*@height
*Returns id of shared memory.
*/
int InitMaze(char** maze, int, int, int);
/*
*Adds avatar to maze. Also used to update it's position.
*@maze is a single array of characters (interpreted as 2d)
* that has been initialized in InitMaze.
*@a identifying character stored in fd added at position "pos"
*@max_width the width of the maze for indexing purposes
*
*/
void AddAvatar(char* maze, myAvatar* a, int max_width);
/*
*Clears a given point in maze.
*@maze is a single array of characters (interpreted as 2d)
* that has been initialized in InitMaze.
*@a provides position "prev" at which a space character is written
*@max_width the width of the maze for indexing purposes
*
*/
void DelAvatar(char* maze, myAvatar* a, int max_width);
/*
*Checks if dead end conditions met, then blocks path.
*Conditions:
*1. avatar is backtracking (has already been to next position)
*2. no avatars will be trapped (was most recent visitor to next position)
*Does nothing when DelAvatar is called.
*
*@maze is a single array of characters (interpreted as 2d)
* that has been initialized in InitMaze.
*@a provides "pos"-to check if avatar is backtracking
* and "prev"-the location to write an x to if backtracking
*@max_width the width of the maze for indexing purposes
*
*/
bool AddMark(char* maze, myAvatar* a, int max_width);
/*
*Checks that last move was unsuccessful, then adds wall in that direction.
*@maze is a single array of characters (interpreted as 2d)
* that has been initialized in InitMaze.
*@a provides "pos", "lastMoveSuccess" and "lastMoveDir"
*@max_width the width of the maze for indexing purposes
*
*/
void AddWall(char* maze, myAvatar* a, int max_width);
/*
*Checks whether suggested move is blocked by wall or mark.
*@maze is a single array of characters (interpreted as 2d)
* that has been initialized in InitMaze.
*@a provides "pos"-the suggested move.
*@max_width the width of the maze for indexing purposes
*Returns true if suggested move is valid.
*/
bool isKnown(char* maze, myAvatar* a, int width);
/*
*Clears the terminal and prints maze (interpreted as 2d rather than string).
*@maze is a single array of characters (interpreted as 2d)
* that has been initialized in InitMaze.
*@width
*@height
*
*/
void PrintMaze(char*, int, int);

#endif
