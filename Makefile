CC = g++
SRC = $(wildcard src/*.cpp)
BUILD = build/test.exe

debug:
	$(CC) $(SRC) -o $(BUILD) -g

release:
	$(CC) $(SRC) -o $(BUILD) -Ofast

