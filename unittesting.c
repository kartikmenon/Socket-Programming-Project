/* ========================================================================== */
/* File: unittesting.c
 *
 * Project name: Amazing Project
 * Authors: Kartikeya Menon, Zachary Kratochvil, Allison Wang
 *
 */
/* ========================================================================== */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

// Useful MACROS for controlling the unit tests.

// each test should start by setting the result count to zero

#define START_TEST_CASE  int rs=0

// check a condition and if false print the test condition failed
// e.g., SHOULD_BE(dict->start == NULL)
// note: the construct "#x" below is the sstringification preprocessor operator that
//       converts the argument x into a character string constant

#define SHOULD_BE(x) if (!(x))  {rs=rs+1; \
printf("Line %d test [%s] Failed\n", __LINE__,#x); \
}


// return the result count at the end of a test

#define END_TEST_CASE return rs

/* Avatar constants */
#define M_WEST           0
#define M_NORTH          1
#define M_SOUTH          2
#define M_EAST           3
#define M_NUM_DIRECTIONS 4
#define M_NULL_MOVE      8

typedef struct XYPos
{
    uint32_t x;
    uint32_t y;
} XYPos;

typedef struct myAvatar
{
    int fd;
    XYPos pos;
    XYPos prev;             // prev position
    int lastMoveSuccess;    // if we went anywhere
    int lastMoveDir;        // where the last move was
    
} myAvatar;

void AddAvatar(char* maze, myAvatar* a, int width) {
    //account for borders
    width = width*2 + 1;
    
    int x = 2*a->pos.x + 1;
    int y = 2*a->pos.y + 1;
    
    // add avatar position to maze
    maze[width * y + x] = (char)(((int)'0')+a->fd);
    
}

void DelAvatar(char* maze, myAvatar* a, int width) {
    //account for borders
    width = width*2 + 1;
    
    int x = 2*a->prev.x + 1;
    int y = 2*a->prev.y + 1;
    
    // delete avatar at position in maze
    maze[width * y + x] = ' ';
    
}

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


#define RUN_TEST(x, y) if (!x()) {              \
printf("Test %s passed\n", y);              \
} else {                                        \
printf("Test %s failed\n", y);              \
cnt = cnt + 1;                              \
}


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


int socketConnect(int sockfd, struct sockaddr_in servaddr, char *IP, \
                  int mazePort) {
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "Could not create socket.\n");
    return 0;   // false
  }
    
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(IP);
  servaddr.sin_port = htons(mazePort);
    
  int checkConnect;
  checkConnect = connect(sockfd, (struct sockaddr*) &servaddr, \
			 sizeof(servaddr));
  if (checkConnect < 0) {
    fprintf(stderr, "Could not connect to server.\n");
    return 0;   // false
  }
    
  return 1;   // if checks passed, socket created and connected to server.
    
}


// Avatar Functions

myAvatar* MakeAvatar1() {
    myAvatar *a = malloc(sizeof(myAvatar));
    a->pos.x = 0;
    a->pos.y = 0;
    a->fd = 0;
    a->lastMoveSuccess = 1; // a success
    a->lastMoveDir = 1; // was going north
    a->prev.x = 0;
    a->prev.y = 1;
    return a;
    
}

myAvatar* MakeAvatar2() {
    myAvatar *a2 = malloc(sizeof(myAvatar));
    a2->pos.x = 2;
    a2->pos.y = 1;
    a2->fd = 1;
    a2->lastMoveSuccess = 0; // not a success
    a2->lastMoveDir = 0; // was going west
    a2->prev.x = 2;
    a2->prev.y = 1;
    
    return a2;
}

// Test SocketControl : Make sure fails if sockfd < 0

int TestSocketControl() {
    START_TEST_CASE;
    
    char *IP = (char*)calloc(20, sizeof(char));
    int mazePort = 500;
    strncpy(IP, "129.170.213.200", 15);
    int sockfd;
    struct sockaddr_in servaddr;
    
    sockfd = -1;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = mazePort;
    
    int result = socketConnect(sockfd, servaddr, IP, mazePort);
    SHOULD_BE(result == 0);
    END_TEST_CASE;
}

// Test SocketControl 2 : Make sure fail if mazeport not valid

int TestSocketControl2() {
    START_TEST_CASE;
    char *IP = (char*)calloc(20, sizeof(char));
    int mazePort = 500;
    strncpy(IP, "129.170.213.200",15);
    int sockfd;
    struct sockaddr_in servaddr;
    
    sockfd = 1;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = mazePort;
    
    int result = socketConnect(sockfd, servaddr, IP, mazePort);
    SHOULD_BE(result == 0);
    END_TEST_CASE;
}

// Test CheckLastMove:

int TestCheckLastMove() {
    START_TEST_CASE;
    myAvatar* a = MakeAvatar1();
    int result = checkLastMove(a);
    
    SHOULD_BE(result != 0);
    
    free(a);
    END_TEST_CASE;
}

// Test CheckLastMove2:

int TestCheckLastMove2() {
    START_TEST_CASE;
    myAvatar* a = MakeAvatar2();
    int result = checkLastMove(a);
    
    SHOULD_BE(result == 0);
    
    free(a);
    
    END_TEST_CASE;
}

// Test GetMove:

int TestGetMove() {
    START_TEST_CASE;
    myAvatar* a = MakeAvatar1();
    int result = getMove(a);
    
    SHOULD_BE(result == 0);     // west
    
    free(a);
    END_TEST_CASE;
}

// Test GetMove2:

int TestGetMove2() {
    START_TEST_CASE;
    myAvatar* a = MakeAvatar2();
    int result = getMove(a);
    
    SHOULD_BE(result == 1);     //north
    
    free(a);
    
    END_TEST_CASE;
}

// Processes
char* CreateMaze() {
    //account for borders
    int width = 7;
    int height = 5;
    
    // Create shared memory
    char * maze = malloc(width*height);
    
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
    
    return maze;
}


myAvatar* MakeAvatar1a() {
    myAvatar *a = malloc(sizeof(myAvatar));
    a->pos.x = 0;
    a->pos.y = 0;
    a->fd = 1;
    return a;
    
}


// Test InitMaze: Correct amount of rows

int TestInitMaze() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[2] == ' ');
    SHOULD_BE(maze[3] == '-');
    SHOULD_BE(maze[4] == ' ');
    SHOULD_BE(maze[5] == '-');
    SHOULD_BE(maze[6] == ' ');
    SHOULD_BE(maze[7] == '|');
    SHOULD_BE(maze[14] == ' ');
    free(maze);
    END_TEST_CASE;
}

// Test InitMaze: Correct amount of columns

int TestInitMaze2() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[7] == '|');
    SHOULD_BE(maze[14] == ' ');
    SHOULD_BE(maze[21] == '|');
    SHOULD_BE(maze[28] == ' ');
    free(maze);
    
    END_TEST_CASE;
}

// Test AddAvatar: Add avatar

int TestAddAvatar() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    myAvatar *a = MakeAvatar1();
    AddAvatar(maze,a,3);
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[7] == '|');
    SHOULD_BE(maze[8] == '0');
    free(a);
    free(maze);
    
    END_TEST_CASE;
}

// Test AddAvatar2: Add existant avatar

int TestAddAvatar2() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    myAvatar *a = MakeAvatar1();
    myAvatar *a2 = MakeAvatar1a();
    AddAvatar(maze,a,3);
    AddAvatar(maze,a2,3);
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[7] == '|');
    SHOULD_BE(maze[8] == '1');
    free(a);
    free(a2);
    free(maze);
    
    END_TEST_CASE;
}

// Test DelAvatar: Del Avatar

int TestDelAvatar() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    myAvatar *a = MakeAvatar2();
    AddAvatar(maze,a,3);
    DelAvatar(maze,a,3);
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[7] == '|');

    SHOULD_BE(maze[8] == ' ');
    free(a);
    free(maze);
    
    END_TEST_CASE;
}

// Test DelAvatar2: Del nonexistant avatar

int TestDelAvatar2() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    myAvatar *a = MakeAvatar1();
    DelAvatar(maze,a,3);
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[7] == '|');
    SHOULD_BE(maze[8] == ' ');
    free(a);
    free(maze);
    
    END_TEST_CASE;
}

// Test AddWall: Add wall

int TestAddWall() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    myAvatar *a = MakeAvatar2();
    AddAvatar(maze,a,3);
    AddWall(maze,a,3);
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[7] == '|');
    SHOULD_BE(maze[25] == '|');
    free(a);
    free(maze);
    
    END_TEST_CASE;
}

// Test AddWall2: Add already existing wall

int TestAddWall2() {
    START_TEST_CASE;
    char *maze = CreateMaze();
    myAvatar *a = MakeAvatar2();
    AddAvatar(maze,a,3);
    AddWall(maze,a,3);
    AddWall(maze,a,3);
    SHOULD_BE(maze[0] == ' ');
    SHOULD_BE(maze[1] == '-');
    SHOULD_BE(maze[7] == '|');
    SHOULD_BE(maze[25] == '|');
    free(a);
    free(maze);
    
    END_TEST_CASE;
}



int main(int argc, char** argv) {
    int cnt = 0;
    TestDelAvatar();
    RUN_TEST(TestSocketControl, "Test SocketControl 1\n");
    RUN_TEST(TestSocketControl2, "Test SocketControl 2\n");
    RUN_TEST(TestCheckLastMove, "Test CheckLastMove 1\n");
    RUN_TEST(TestCheckLastMove2, "Test CheckLastMove 2\n");
    RUN_TEST(TestGetMove, "Test GetMove 1\n");
    RUN_TEST(TestGetMove2, "Test GetMove 2\n");
    RUN_TEST(TestInitMaze, "Test InitMaze 1\n");
    RUN_TEST(TestInitMaze2, "Test InitMaze 2\n");
    RUN_TEST(TestAddAvatar, "Test Add Avatar 1\n");
    RUN_TEST(TestAddAvatar2, "Test Add Avatar 2\n");
    RUN_TEST(TestDelAvatar, "Test Delete Avatar 1\n");
    RUN_TEST(TestDelAvatar2, "Test Delete Avatar 2\n");
    RUN_TEST(TestAddWall, "Test Add Wall\n");
    RUN_TEST(TestAddWall2, "Test Add Wall 2\n");

    
    if (!cnt) {
        printf("All passed!\n");
        return 0;
    } else {
        printf("Some fails!\n");
        return 1;
    }
    return 0;
}




