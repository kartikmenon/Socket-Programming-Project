/* ========================================================================== */
/* File: header.h
 *
 * Project name: Amazing Project
 * Authors: Kartikeya Menon, Zachary Kratochvil, Allison Wang
 *
 */
/* ========================================================================== */
#ifndef UTILS_H
#define UTILS_H

// ---------------- Prerequisites e.g., Requires "math.h"
#include <stdio.h>                           // fprintf
#include <stdlib.h>                          // exit
#include "./amazing.h"

// ---------------- Constants
#define SHMEM_KEY 50321

// ---------------- Structures/Types


typedef struct Cell {
    // moves[0]: west
    // moves[1]: north
    // moves[2]: south
    // moves[3]: east
    // Will contain 0 or 1, 0 for wall or 1 for can move
    int moves[4];
    int visited;
} Cell;


typedef struct myAvatar
{
    int fd;
    XYPos pos;
    XYPos prev;             // prev position
    int lastMoveSuccess;    // if we went anywhere
    int lastMoveDir;        // where the last move was
    
} myAvatar;



// ---------------- Public Variables
//Cell* maze;   // a global to render graphics

// ---------------- Prototypes/Macros

// Print s with the source file name and the current line number to fp.
#define LOG(fp,s)  fprintf((fp), "Log: [%s:%d] %s.\n", __FILE__, __LINE__, (s))

// For log file
#define OUTPUT(fp, s)  fprintf((fp), (s))

// Print error to fp and exit if s evaluates to false.
#define ASSERT_FAIL(fp,s) do {                                          \
if(!(s))   {                                                    \
fprintf((fp), "Error: [%s:%d] assert failed.\n", __FILE__, __LINE__); \
exit(EXIT_FAILURE);                                         \
}                                                               \
} while(0)

// Print error to fp and exit if s is NULL.
#define MALLOC_CHECK(fp,s)  do {                                        \
if((s) == NULL)   {                                             \
fprintf((fp), "Error: [%s:%d] out of memory.\n", __FILE__, __LINE__); \
exit(EXIT_FAILURE);                                         \
}                                                               \
} while(0)

#endif // UTILS_H