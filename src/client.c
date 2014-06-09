/* ========================================================================== */
/* File: client.c
 *
 * Project name: Amazing Project
 * Component name: Called by AMStartup to guide avatars
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include "../utils/header.h"
#include "../utils/AMStartup.h"
#include "../utils/amazing.h"
#include "../utils/lefthandrule.h"
#include "../utils/maze.h"

int VERBOSE;
int GRAPHICS;
/* ==========================================================================
 * Check the arguments passed from AMStartup
 * ========================================================================== */
void validate(int argc, char *argv[]) {
    
    if (argc == 12 && !strncmp(argv[11], "-v", strlen(argv[11]))) {
        fprintf(stdout, "Verbose - client will print moves.\n");
        VERBOSE = 1;
    }
    
    else if (argc == 12 && !strncmp(argv[11], "-g", strlen(argv[11]))) {
        fprintf(stdout, "Graphics - client will print moves.\n");
        GRAPHICS = 1;
    }
    
    else if (argc == 12 && !strncmp(argv[11], "-vg", strlen(argv[11]))) {
        fprintf(stdout, "Verbose with graphics- client will print moves.\n");
        VERBOSE = 1;
        GRAPHICS = 1;
    }
    
    else if (argc == 12 && !strncmp(argv[11], "-gv", strlen(argv[11]))) {
        fprintf(stdout, "Verbose with graphics- client will print moves.\n");
        VERBOSE = 1;
        GRAPHICS = 1;
    }
    
    
    else if (argc != 11) {
        fprintf(stderr, "Wrong number of arguments.\n");
        exit(1);
    }
    
    int nAvatars = atoi(argv[2]);
    int difficulty = atoi(argv[3]);
    int iD = atoi(argv[1]);
    
    // Check that they are all in bounds.
    if (nAvatars > 9 || nAvatars < 2) {
        fprintf(stderr, "Number of avatars out of range. Need 2 < n < 9.\n");
        exit(1);
    }
    
    if (difficulty > 9 || difficulty < 0) {
        fprintf(stderr, "Difficulty out of range. Need 0 < n < 9.\n");
        exit(1);
    }
    
    if (iD > nAvatars) {
        fprintf(stderr, "ID not valid.\n");
        exit(1);
    }
    
    FILE *fp;
    fp = fopen(argv[6], "r");
    if (!fp) {
        fprintf(stderr, "Unable to access or open log file.\n");
        exit(1);
    }
    fclose(fp);
    
}

/* ==========================================================================
 * Do all the socket stuff to connect local to server
 * ========================================================================== */
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

int main(int argc, char *argv[]) {

    VERBOSE = 0;
    GRAPHICS = 0;
    validate(argc, argv);
    
    int avatarID = atoi(argv[1]);
    FILE *logFile;
    logFile = fopen(argv[6], "a+");
    char *IP = (char*)calloc(strlen(argv[4]) + 1, sizeof(char));
    MALLOC_CHECK(stderr, IP);
    strncpy(IP, argv[4], strlen(argv[4]));
    
    
    myAvatar *avatar;
    int slobAvatar = 0;     // the one who doesn't move in maze solver

    int mazePort = atoi(argv[5]);
    
    int width = atoi(argv[7]);
    int height = atoi(argv[8]);
    int shmID = atoi(argv[9]);
    char *maze = (char*) shmat(shmID, 0, 0);
    if (maze == (void *)-1 && errno == EACCES) {

        fprintf(stderr, "shmat failed EACCES\n");
        exit(1);
    }
    if (maze == (void *)-1 && errno == EINVAL) {

        fprintf(stderr, "shmat failed EINVAL\n");
        exit(1);
    }
    if (maze == (void *)-1 && errno == ENOMEM) {

        fprintf(stderr, "shmat failed ENOMEM\n");
        exit(1);
    }

    // Server connection stuff
    int sockfd;
    struct sockaddr_in servaddr;
    
    // Create and check status of server
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        free(IP);
        fprintf(stderr, "Could not create socket.\n");
        return 0;   // false
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(IP);
    servaddr.sin_port = htons(mazePort);
    
    // Connect to server
    if (connect(sockfd, (struct sockaddr*) &servaddr, \
                sizeof(servaddr)) < 0) {
        fprintf(stderr, "Unable to connect to server. Exiting.\n");
        free(IP);
        return 0;   // false
    }
    
    // Fire off initial AM_AVATAR_READY
    AM_Message *sendmsg = (AM_Message*)calloc(1, sizeof(AM_Message));
    MALLOC_CHECK(stderr, sendmsg);
    
    sendmsg->type = htonl(AM_AVATAR_READY);
    sendmsg->avatar_ready.AvatarId = htonl(avatarID);  // pack information
    
    // Send ready message to server containing avatar ID
    send(sockfd, sendmsg, sizeof(AM_Message), 0);
    
    // Make a message to get back from server
    AM_Message *receivemsg = (AM_Message*)calloc(1, sizeof(AM_Message));
    MALLOC_CHECK(stderr, receivemsg);

    int firstMove = 1;
    int turn = -1;
    // Start while loop that checks all server messages and updates
    // the avatar's position until either error message or AM_SOLVED received
    for ( ;; ) {
        
        // Zero out the message pointers so they can be re-used
        memset(receivemsg, 0, sizeof(AM_Message));
        memset(sendmsg, 0, sizeof(AM_Message));
        
        // Receive the message and make sure it's not empty
        if (recv(sockfd, receivemsg, sizeof(AM_Message), 0) == 0) {
            fprintf(stderr, "Server terminated early.\n");
            OUTPUT(logFile, "Empty message type. Exiting...\n");
            break;
        }
        
        // Check for server error
        if (IS_AM_ERROR(receivemsg->type)) {
            fprintf(stderr, "Error mask detected error.\n");
        }
        
        // Following if statements will check for all possible server errors
        // In all cases, print to stderr, log the error to output file
        // and break to free resources and return
        
        if (ntohl(receivemsg->type) == AM_UNKNOWN_MSG_TYPE) {
            fprintf(stderr, "AM_UNKNOWN_MSG_TYPE\n");
            OUTPUT(logFile, "Unknown message type. Exiting...\n");
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_NO_SUCH_AVATAR) {
            fprintf(stderr, "AM_NO_SUCH_AVATAR\n");
            OUTPUT(logFile, "No such avatar error. Exiting...\n");
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_UNEXPECTED_MSG_TYPE) {
            fprintf(stderr, "AM_UNEXPECTED_MSG_TYPE\n");
            OUTPUT(logFile, "Unexpected message type. Exiting...\n");
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_AVATAR_OUT_OF_TURN) {
            fprintf(stderr, "AM_AVATAR_OUT_OF_TURN\n");
            OUTPUT(logFile, "Avatar out of turns message type. Exiting...\n");
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_TOO_MANY_MOVES) {
            fprintf(stderr, "AM_AVATAR_TOO_MANY_MOVES\n");
            OUTPUT(logFile, "Avatar too many moves message type. Exiting...\n");
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_SERVER_TIMEOUT) {
            fprintf(stderr, "AM_SERVER_TIMEOUT\n");
            OUTPUT(logFile, "Server timeout message type. Exiting...\n");
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_SERVER_DISK_QUOTA) {
            fprintf(stderr, "AM_SERVER_DISK_QUOTA\n");
            OUTPUT(logFile, "Server disk quota message type. Exiting...\n");
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_SERVER_OUT_OF_MEM) {
            fprintf(stderr, "AM_SERVER_OUT_OF_MEM\n");
            OUTPUT(logFile, "Server out of mem message type. Exiting...\n");
            break;
        }
        
        // Huzzah! Output success to log file.
        if (ntohl(receivemsg->type) == AM_MAZE_SOLVED) {
            
            // Only output this stuff once.
            if (avatarID == slobAvatar) {
                fprintf(stdout, "SOLVED!!!\n");
                fprintf(logFile, "Maze solved with %d avatars on %d"
                        " difficulty in %d moves. The Hash key is: %d.\n", ntohl(receivemsg->maze_solved.nAvatars),\
                        ntohl(receivemsg->maze_solved.Difficulty), ntohl(receivemsg->maze_solved.\
                        nMoves), ntohl(receivemsg->maze_solved.Hash));
            }
            
            break;
        }
        
        if (ntohl(receivemsg->type) == AM_AVATAR_TURN) {
            // Only look at messages from the relevant reply messages
            if (ntohl(receivemsg->avatar_turn.TurnId) == avatarID) {
                
                if (firstMove) {
                    // Initialize the avatar.
                    avatar = (myAvatar*)calloc(1, sizeof(myAvatar));
                    MALLOC_CHECK(stderr, avatar);
                    avatar->fd = avatarID;
                    avatar->lastMoveDir = M_NORTH;  // everyone goes north initially
                    
                    // Initially, avatar's previous position and current position
                    // are the same. Retrieve these from server.
                    avatar->pos.x = ntohl(receivemsg->avatar_turn.Pos[avatarID].x);
                    avatar->pos.y = ntohl(receivemsg->avatar_turn.Pos[avatarID].y);
                    
                    // No previous initially.
                    
                    // Send the avatar north.
                    sendmsg->type = htonl(AM_AVATAR_MOVE);
                    sendmsg->avatar_move.AvatarId = htonl(avatarID);
                    sendmsg->avatar_move.Direction = htonl(M_NORTH);
                    
                    if (VERBOSE) {
                        fprintf(stdout, "Avatar %d: Initial pos (%d, %d) moved NORTH\n", avatarID,\
                                avatar->pos.x, avatar->pos.y);
                    }
                    // Send ready message to server containing avatar ID
                    send(sockfd, sendmsg, sizeof(AM_Message), 0);
                    turn = 1;
                    firstMove = 0;

                    if (GRAPHICS) {
                        //initialize graphics
                        AddAvatar(maze,avatar,width);
                        PrintMaze(maze, width, height);
                    }
                    
                    continue;
                    
                }
                
                else {  // not first move
                    
                    // If it's the avatar standing still, arbitrarily picked as
                    // avatar with ID = 0, then always stand still.
                    if (avatarID == slobAvatar) {
                        sendmsg->type = htonl(AM_AVATAR_MOVE);
                        sendmsg->avatar_move.AvatarId = htonl(avatarID); // should be 0
                        sendmsg->avatar_move.Direction = htonl(M_NULL_MOVE);
                        send(sockfd, sendmsg, sizeof(AM_Message), 0);
                        turn++;
                        continue;
                    }
                    
                    // Make the previous the current position
                    avatar->prev.x = avatar->pos.x;
                    avatar->prev.y = avatar->pos.y;
                    turn++;
                    // Make current position the new one from the server
                    int newX = ntohl(receivemsg->avatar_turn.Pos[avatarID].x);
                    int newY = ntohl(receivemsg->avatar_turn.Pos[avatarID].y);
                    avatar->pos.x = newX;
                    avatar->pos.y = newY;
                    
                    // See if the slob and the current avatar are on the same spot.
                    int deltaX = avatar->pos.x - ntohl(receivemsg->avatar_turn.Pos[slobAvatar].x);
                    int deltaY = avatar->pos.y - ntohl(receivemsg->avatar_turn.Pos[slobAvatar].y);
                    int checkSameCell = 0;
                    
                    if ((deltaY == 0) && (deltaX == 0)) {
                        checkSameCell = 1;
                    }
                    // If avatar is on same spot as slob, slob has been found.
                    // Don't move the other avatar.
                    if (checkSameCell) {
                        sendmsg->type = htonl(AM_AVATAR_MOVE);
                        sendmsg->avatar_move.AvatarId = htonl(avatarID);
                        sendmsg->avatar_move.Direction = htonl(M_NULL_MOVE);
                        send(sockfd, sendmsg, sizeof(AM_Message), 0);
                        turn++;
                        continue;
                    }
                    
                    // Avatar is not slob and has not yet met slob.
                    else {
                        
                        // See if the last move was productive
                        int lastMove = checkLastMove(avatar);
                        avatar->lastMoveSuccess = lastMove;

                        //update graphics based on last move

                        if(!lastMove){
                            AddWall(maze,avatar,width);
                        }
                        else{
                            //DelAvatar(maze,avatar,width);//comment out to see history
                            if(!AddMark(maze,avatar,width))//blocks square if test passes
                                AddAvatar(maze,avatar,width);
                        }
                        
                        if (GRAPHICS) {
                            PrintMaze(maze,width,height);
                        }
                        
                        
                        

                        //prep next move
                        int nextMove = getMove(avatar);
                        avatar->lastMoveDir = nextMove;
                        while(isKnown(maze, avatar, width)){
                            avatar->lastMoveSuccess = false;
                            nextMove = getMove(avatar);
                            avatar->lastMoveDir = nextMove;
                        }
                        
                        turn++;
                        sendmsg->type = htonl(AM_AVATAR_MOVE);
                        sendmsg->avatar_move.AvatarId = htonl(avatarID);
                        sendmsg->avatar_move.Direction = htonl(nextMove);
                        
                        if (VERBOSE) {
                            fprintf(stdout, "======== Turn %d ========\n", turn);
                            if (nextMove == M_NORTH) {
                                fprintf(stdout, "Avatar %d: Moved NORTH from (%d, %d).\n", avatarID,\
                                        avatar->pos.x, avatar->pos.y);
                            }
                            if (nextMove == M_SOUTH) {
                                fprintf(stdout, "Avatar %d: Moved SOUTH from (%d, %d).\n", avatarID,\
                                        avatar->pos.x, avatar->pos.y);
                            }
                            
                            if (nextMove == M_WEST) {
                                fprintf(stdout, "Avatar %d: Moved WEST from (%d, %d).\n", avatarID,\
                                        avatar->pos.x, avatar->pos.y);
                            }
                            
                            if (nextMove == M_EAST) {
                                fprintf(stdout, "Avatar %d: Moved EAST from (%d, %d).\n", avatarID,\
                                        avatar->pos.x, avatar->pos.y);
                            }
                            fprintf(stdout, "\n");
                        }
                        
                        send(sockfd, sendmsg, sizeof(AM_Message), 0);
                    }
                    
                }
            }
            
        }
        
    }
    shmctl(shmID, IPC_RMID, NULL);
    free(sendmsg); free(receivemsg); free(IP); free(avatar);
    fclose(logFile);
    close(sockfd);
    return 1;
    
}