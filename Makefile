CC = g++
SRC = $(wildcard src/*.cpp)
BUILD = build/test.exe

debug:
	$(CC) $(SRC) -o $(BUILD) -g

standard:
	$(CC) $(SRC) -o $(BUILD)

release:
	$(CC) $(SRC) -o $(BUILD) -Ofast
