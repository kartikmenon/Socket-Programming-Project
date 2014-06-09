## Socket Programming and Shared Memory Project

### How to use:
Run 'make' in the project's root directory to make AMStartup and client executables.
Run with: AMStartup [nAvatars] [Difficulty] [Server Name]

### How it Works
This client connects to a server, located on pierce.cs.dartmouth.edu and requests a maze according to the difficulty. The server generates the maze and places n number of Avatars inside the maze at random locations. Each Avatar is run as a separate process accessing the same shared memory. The client will finish and free resources when the maze is solved. Each calculated move is sent to the server and the server responds with the x-y coordinates of the Avatars in the maze after the move.