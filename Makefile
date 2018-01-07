CC = g++
EXE = snkp2p
IDIR = include
SRC = src
OBJ = $(OBJ_DIR)/file_manager.o $(OBJ_DIR)/conn_manager.o $(OBJ_DIR)/p2p.o $(OBJ_DIR)/base64.o $(OBJ_DIR)/main.o $(OBJ_DIR)/parser.o
LIBS = -lpthread
CXXFLAGS += -std=c++11 -Wall -I$(IDIR)
OBJ_DIR = obj

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LIBS) $^ -o $@

$(OBJ_DIR)/file_manager.o: $(SRC)/file_manager.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/parser.o: $(SRC)/parser.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/conn_manager.o: $(SRC)/conn_manager.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/p2p.o: $(SRC)/p2p.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/base64.o: $(SRC)/base64.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/main.o: $(SRC)/main.cpp
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm $(OBJ_DIR)/*.o
