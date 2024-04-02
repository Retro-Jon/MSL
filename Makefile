CC = g++
SRC = $(wildcard src/*.cpp)
BUILD = build/test.exe

debug-linux:
	$(CC) -D LINUX $(SRC) -o $(BUILD) -g

debug-windows:
	$(CC) -D WINDOWS $(SRC) -o $(BUILD) -g

release-linux:
	$(CC) -D LINUX $(SRC) -o $(BUILD) -Ofast

release-windows:
	$(CC) -D WINDOWS $(SRC) -o $(BUILD) -Ofast

