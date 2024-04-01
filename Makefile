CC = g++
SRC = $(wildcard src/*.cpp)
BUILD = build/test.exe

debug:
	$(CC) $(SRC) -o $(BUILD) -g

standard:
	$(CC) $(SRC) -o $(BUILD) -Ofast

unsafe:
	$(CC) $(SRC) -o $(BUILD) -Ofast -fno-signed-zeros -fno-trapping-math -ffast-math -funroll-loops -falign-functions -falign-loops 

