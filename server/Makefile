CC = g++
OBJS = server.o main.o parser.o
CXXFLAGS = -std=c++11 -Wall -c
LIBS = -lpthread
INCLUDE_DIR = "../include"
SRC_DIR = "../src"

all: server

server: $(OBJS)
	$(CC) $(LIBS) $^ -o $@

server.o: server.hpp server.cpp
	$(CC) $(CXXFLAGS) $^

parser.o: $($INCLUDE_DIR + parser.hpp) $(SRC_DIR + parser.cpp)
	$(CC) $(CXXFLAGS) $^

main.o: main.cpp
	$(CC) $(CXXFLAGS) $^

clean:
	rm *.o *.gch snkp2p
