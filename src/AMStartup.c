/* ========================================================================== */
/* File: AMStartup.c
 *
 * Project name: Amazing Project
 * Component name: AMStartup
 *
 * Author: Kartik Menon, Zach Kratochvil, Allison Wang
 * Date: May 2014
 *
 * Input: [AVATAR NUMBER] [DIFFICULTY] [HOST NAME]
 * Output: No official output
 *
 * Pseudocode: Validates arguments, initializes connection to server, 
 * prepares arugments to the client program for each avatar, and
 * forks into separate processes each of which calls client with the prepared,
 * avatar-specific arguments.
 *
 */
/* ========================================================================== */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../utils/amazing.h"
#include "../utils/header.h"
#include "../utils/AMStartup.h"
#include "../utils/maze.h"

// latter half of Amazing_$USER -> _N_D.log
#define nameSize 10

void validate(int argc, char *argv[]) {
    
    if (argc == 5 && !strncmp(argv[4], "-v", strlen(argv[4]))) {
        fprintf(stdout, "Verbose startup.\n");
    }
    
    else if (argc == 5 && !strncmp(argv[4], "-g", strlen(argv[4]))) {
        fprintf(stdout, "Graphics startup.\n");
    }
    
    else if (argc == 5 && !strncmp(argv[4], "-vg", strlen(argv[4]))) {
        fprintf(stdout, "Verbose with graphics startup.\n");
    }
    
    else if (argc == 5 && !strncmp(argv[4], "-gv", strlen(argv[4]))) {
        fprintf(stdout, "Verbose with graphics startup.\n");
    }
    
    else if (argc != 4) {
        fprintf(stderr, "Please supply correct arguments.\n");
        fprintf(stderr, "Usage: ./AMStartup [AVATAR NUMBER]"
                " [DIFFICULTY] [HOST NAME]\n");
        exit(1);
    }
    
    int nAvatars = atoi(argv[1]);
    int difficulty = atoi(argv[2]);
    char *hostName = argv[3];
    
    // Check that they are all in bounds.
    if (nAvatars > 9 || nAvatars < 2) {
        fprintf(stderr, "Number of avatars out of range. Need 2 < n < 9.\n");
        exit(1);
    }
    
    if (difficulty > 9 || difficulty < 0) {
        fprintf(stderr, "Difficulty out of range. Need 0 <= n <= 9.\n");
        exit(1);
    }
    
    if (strncmp(hostName, "pierce.cs.dartmouth.edu", \
                strlen("pierce.cs.dartmouth.edu") + 1)) {
        fprintf(stderr, "Please run maze client on pierce.cs.dartmouth.edu\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    validate(argc, argv);
    int nAvatars = atoi(argv[1]);
    int difficulty = atoi(argv[2]);
    char *hostName = argv[3];
    
    // Start server stuff
    struct sockaddr_in servaddr;
    int sockfd;
    
    // Create socket and check status
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        fprintf(stderr, "Socket creation failure. Exiting.\n");
        exit(1);
    }
    
    // Create the message to sendmsg over
    AM_Message *sendmsg = (AM_Message*)calloc(1, sizeof(AM_Message));
    MALLOC_CHECK(stderr, sendmsg);
    
    sendmsg->type = htonl(AM_INIT);
    sendmsg->init.nAvatars = htonl(nAvatars);
    sendmsg->init.Difficulty = htonl(difficulty);
    
    AM_Message *receive = (AM_Message*)calloc(1, sizeof(AM_Message));
    MALLOC_CHECK(stderr, receive);
    
    // Prepare socket address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    int serverPort = atoi(AM_SERVER_PORT);
    servaddr.sin_port = htons(serverPort);
    
    // Retrieve IP address from hostname
    char *IP = getIPAddress(hostName);
    
    if (!IP) {   // check == 0
        fprintf(stderr, "Hostname failed to resolve to IP.\n");
        free(sendmsg); free(receive);
        exit(1);
    }
    servaddr.sin_addr.s_addr = inet_addr(IP);
    
    
    // Connect to the server. If failed, free resources and exit
    if (connect(sockfd, (struct sockaddr*) &servaddr, \
                sizeof(servaddr)) < 0) {
        fprintf(stderr, "Problem connecting to server.\n");
        free(sendmsg); free(receive); free(IP);
        exit(1);
    }
    
    // Fire off the AM_INIT message
    send(sockfd, sendmsg, sizeof(AM_Message), 0);
    
    if (recv(sockfd, receive, sizeof(AM_Message), 0) == 0) {
        fprintf(stderr, "Server terminated prematurely.\n");
        free(sendmsg); free(receive); free(IP);
        exit(1);
    }
    
    free(sendmsg);
    
    // Get information from received message
    int mazePort = ntohl(receive->init_ok.MazePort);
    int width = ntohl(receive->init_ok.MazeWidth);
    int height = ntohl(receive->init_ok.MazeHeight);
    
    // Find environment variable user from shell
    char* USER = getenv("USER");
    
    // Start creating log file
    char *fileName = (char*)calloc(strlen("Amazing_") + 1 + strlen(USER)\
                                   + nameSize, sizeof(char));
    
    // Construct file name
    strncat(fileName, "Amazing_", strlen("Amazing_"));
    strncat(fileName, USER, strlen(USER));
    strncat(fileName, "_", strlen("_"));
    strncat(fileName, argv[1], strlen(argv[1]));    // nAvatars
    strncat(fileName, "_", strlen("_"));
    strncat(fileName, argv[2], strlen(argv[2]));    // difficulty
    strncat(fileName, ".log\0", strlen(".log\0"));
    
    // ctime()'s return value is exactly 26 characters
    time_t currentTime;
    struct tm* timeInfo;
    char timeString[27];
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    strftime(timeString, sizeof(timeString), "%A: %d:%H:%M:%S", timeInfo);
    
    char *filePath = (char*)calloc(strlen("./LogFiles/") + strlen(fileName)\
                                   + 1, sizeof(char));
    strncat(filePath, "./LogFiles/", strlen("./LogFiles/"));
    strncat(filePath, fileName, strlen(fileName));
    
    // Append mode
    FILE *logFile;
    logFile = fopen(filePath, "a+");
    
    if (!logFile) {
        fprintf(stderr, "Log file failed to create.\n");
        free(filePath); free(fileName); free(receive); free(IP);
        exit(1);
    }
    
    // First line of log file
    fprintf(logFile, "User: %s MazePort: %d Time: %s\n", USER, mazePort, timeString);
    fclose(logFile);
    
    // Allocate!
    char *charMazePort = (char*)calloc(AM_MAX_MESSAGE, sizeof(char));
    sprintf(charMazePort, "%d", mazePort);
    
    int v = 0;
    int g = 0;
    char *nAvatarsString = (char*)calloc(strlen(argv[1]) + 1, sizeof(char));
    char *difficultyString = (char*)calloc(strlen(argv[2]), sizeof(char));
    
    char *verbose;
    if (argc == 5) {
        verbose = (char*)calloc(strlen(argv[4]), sizeof(char));
        strncpy(verbose, argv[4], strlen(argv[4]));
        v = 1;
        if (strstr(argv[4], "-g")) {
            g = 1;
        }
    }

    //initialize shared memory
    //initialize graphics if avatar 0
    int shmID;
    char *maze;
    shmID = InitMaze(&maze, width, height, g);

    
    strncpy(nAvatarsString, argv[1], strlen(argv[1]));
    strncpy(difficultyString, argv[2], strlen(argv[2]));
    
    char *args[12];
    args[0] = "./client";
    args[2] = nAvatarsString;  // nAvatars strcpy?
    args[3] = difficultyString;  // difficulty ditto
    args[4] = IP;
    args[5] = charMazePort;
    args[6] = filePath;
    args[7] = (char*)calloc(10, sizeof(char));
    sprintf(args[7], "%d", width);
    args[8] = (char*)calloc(10, sizeof(char));
    sprintf(args[8], "%d", height);
    args[9] = (char*)calloc(10, sizeof(char));
    sprintf(args[9], "%d", shmID);
    
    if (v) {
        args[11] = verbose;
        args[12] = NULL; // execv req
    }
    else {
        args[11] = NULL;
    }
    
    
    // Create processes for each individual avatar and, begin client for each
    for (int i = 0; i < nAvatars; i++) {
        args[1] = (char*)calloc(1, sizeof(int));    // serves as avatar ID
        sprintf(args[1], "%d", i);
        
        pid_t pid = fork();
        
        // Execute new avatar in each new process
        switch (pid) {
            case 0:
                execv("./client", args);
                break;
                
            case -1:
                fprintf(stderr, "Error: Fork failed to create new process.\n");
                return 1;    // don't continue if fork fails
                
            default:
                break;  // parent
        }
        free(args[1]);
        args[1] = NULL;
        
    }
    free(args[9]);
    free(args[7]);
    free(args[8]);
    free(charMazePort);
    free(nAvatarsString);
    free(difficultyString);
    free(IP);
    free(fileName);
    free(filePath);
    free(receive);
    
    if (v) {
        free(verbose);
    }
    
    close(sockfd);
    
}

/* ==========================================================================
 * Resolve the IP address from the hostname
 *
 * *** Content ***
 * Adapted from www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
 * ========================================================================== */
char* getIPAddress(char *hostName) {
    struct hostent *host;
    struct in_addr addr_list;
    char *IP = (char*)calloc(AM_MAX_MESSAGE, sizeof(char));
    
    host = gethostbyname(hostName);
    if (!hostName) {
        fprintf(stderr, "Could not resolve host name.\n");
        return NULL;
    }
    memcpy(&addr_list, host->h_addr_list[0], sizeof(struct in_addr));
    
    // IP will be no larger than max size sent back by server
    strncpy(IP, inet_ntoa(addr_list), AM_MAX_MESSAGE);
    
    return IP;
    
}