# Makefile for Amazing project

CC = gcc
CFLAGS = -Wall -pedantic -std=c11 -ggdb -D_SVID_SOURCE -lpthread

CLIENT=client
SRCS=./src/client.c ./src/lefthandrule.c ./src/graphics.c
INITSRC=./src/AMStartup.c ./src/graphics.c
INIT=AMStartup
HEADERS=./utils/amazing.h ./utils/lefthandrule.h ./utils/AMStartup.h ./utils/maze.h

maze:
	make $(INIT)
	make $(CLIENT)

$(CLIENT): $(SRCS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

$(INIT): $(INITSRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(INITSRC)

test: ./unittesting.c
	$(CC) $(CFLAGS) -o test ./unittesting.c

clean:
	rm -f *~f
	rm -f ./client
	rm -f ./AMStartup
	rm -f ./test

cleanlogs:
	cd ./LogFiles/; rm -f Amazing_kmenon*

