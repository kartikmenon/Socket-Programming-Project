/* ========================================================================== */
/* File: graphics.c
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
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>

// ---------------- Local includes  e.g., "file.h"
#include "../utils/AMStartup.h"
#include "../utils/maze.h"
#include "../utils/amazing.h"
#include "../utils/header.h"

// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types

// ---------------- Private variables

// ---------------- Private prototypes





/* ==========================================================================
 * Adds an avatar in ASCII according to it's ID into its position in the maze
 * ========================================================================== */
void AddAvatar(char* maze, myAvatar* a, int width) {
  //account for borders
  width = width*2 + 1;
  
  int x = 2*a->pos.x + 1;
  int y = 2*a->pos.y + 1;

  // add avatar position to maze 
  maze[width * y + x] = (char)(((int)'0')+a->fd);

}

/* ==========================================================================
 * Take the avatar at it's position and reset the ASCII to a space
 * ========================================================================== */
void DelAvatar(char* maze, myAvatar* a, int width) {
  //account for borders
  width = width*2 + 1;

  int x = 2*a->prev.x + 1;
  int y = 2*a->prev.y + 1;

  // delete avatar at position in maze
  maze[width * y + x] = ' ';

}

/* ==========================================================================
 * Function to block off dead ends
 *
 * *** Content ***
 * If an avatar has already visited a spot, know it's backtracking out of a
 * dead end. Replace the space in the maze char with a 'visited' x mark.
 * ========================================================================== */
bool AddMark(char* maze, myAvatar* a, int width) {
  //account for borders
  width = width*2 + 1;

  int x = 2*a->pos.x + 1;
  int y = 2*a->pos.y + 1;

  //check whether to add spot already has this avatar
  if(maze[width * y + x] == (char)(((int)'0')+a->fd)){
    //set mark to block previous spot

    x = 2*a->prev.x + 1;
    y = 2*a->prev.y + 1;

    maze[width * y + x] = 'x';
    return true;
  }
  return false;
}

/* ==========================================================================
 * Add dashes and |'s to the maze char depending on the avatar's move success
 * ========================================================================== */
void AddWall(char* maze, myAvatar* a, int width) {
  //account for borders
  width = width*2 + 1;

  // if ran into a wall                          
  if (a->lastMoveSuccess == 0) {

    int x = 2*a->pos.x + 1;
    int y = 2*a->pos.y + 1;


    if (a->lastMoveDir == 0) {
      x = x - 1;
      maze[width*y + x] = 124;
    }

    // if north                                  
    if (a->lastMoveDir == 1) {
      y = y - 1;
      maze[width*y + x] = 45;
    }

    // if south                                  
    if (a->lastMoveDir == 2) {
      y = y + 1;
      maze[width*y + x] = 45;
    }
    // if east
    if (a->lastMoveDir == 3) {
      x = x + 1;
      maze[width*y + x] = 124;
    }
     
  }
}


/* ==========================================================================
 * See if the moves in the spot the avatar is in have been explored
 * ========================================================================== */
bool isKnown(char* maze, myAvatar* a, int width) {
  //account for borders
  width = width*2 + 1;

  int x = 2*a->pos.x + 1;
  int y = 2*a->pos.y + 1;


  if (a->lastMoveDir == 0) {
    x--;
    if(maze[width*y + x] == 124)
      return true;
    x--;
    if(maze[width*y + x] == 'x')
      return true;
  }

  // if north                                  
  if (a->lastMoveDir == 1) {
    y--;
    if(maze[width*y + x] == 45)
      return true;
    y--;
    if(maze[width*y + x] == 'x')
      return true;
  }

  // if south                                  
  if (a->lastMoveDir == 2) {
    y++;
    if(maze[width*y + x] == 45)
      return true;
    y++;
    if(maze[width*y + x] == 'x')
      return true;
  }
  // if east
  if (a->lastMoveDir == 3) {
    x++;
    if(maze[width*y + x] == 124)
      return true;
    x++;
    if(maze[width*y + x] == 'x')
      return true;
  }

  return false;
}

/* ==========================================================================
 * Loop and print maze char,
 * ========================================================================== */
void PrintMaze(char *maze, int width, int height) {
  //account for borders
  width = width*2 + 1;
  height = height*2 + 1;

  system("clear");
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      printf("%c", maze[width*j + i]);
    }
    printf("\n");
  }
}

/* ==========================================================================
 * Create maze borders and start filling maze char array.
 * ========================================================================== */
int InitMaze(char** mazeIn, int widthIn, int heightIn, int graphics) {
  //account for borders
  int width = widthIn*2 + 1;
  int height = heightIn*2 + 1;

  // Create shared memory
  key_t key = SHMEM_KEY;
  int shmid = shmget(key, sizeof(char) * width * height, 0666 | IPC_CREAT);
    
  if (shmid == -1) {
    fprintf(stderr, "shmget failed\n");
    exit(1);
  }

  *mazeIn = (char*) shmat(shmid, 0, 0);
  char *maze = *mazeIn;
  if (maze == (void *)-1) {
    fprintf(stderr, "shmat failed\n");
    exit(1);
  }
   
  // top border
  int last = 0;
  for (int i = 0; i < width - 2; i += 2) {
    maze[i] = 32;
    maze[i+1] = 45;
    last = i+2;
  }
  maze[last] = 32;
 
  // side borders
  for (int i = 1; i < height - 1; i++) {

    if (i%2 == 0)
      maze[width*i + 0] = 32;
    else
      maze[width*i + 0] = 124;

    for(int j = 1; j < width - 1; j++) {
      maze[width*i + j] = 32;
    }
   
    if (i%2 == 0)
      maze[width*i + width - 1] = 32;
    else
      maze[width*i + width - 1] = 124;
  }

  // bottom border
  last = 0;
  for (int i = 0; i < width - 2; i += 2) {
    maze[width*(height-1) + i] = 32;
    maze[width*(height-1) + i + 1] = 45;
    last = i+2;
  }
  maze[width*(height-1) + last] = 32;
  
    if (graphics) {
        printf("\n");
        PrintMaze(maze, widthIn, heightIn);
    }
  
    return shmid;
}