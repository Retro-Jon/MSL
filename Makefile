CC = g++
SRC = $(wildcard src/*.cpp)
BUILD_LINUX = build/msol
BUILD_WIN = build/msol.exe

debug-linux:
	$(CC) -D LINUX -D DEBUG $(SRC) -o $(BUILD_LINUX) -g

debug-windows:
	$(CC) -D WINDOWS -D DEBUG $(SRC) -o $(BUILD_WIN) -g

release-linux:
	$(CC) -D LINUX $(SRC) -o $(BUILD_LINUX) -Ofast

release-windows:
	$(CC) -D WINDOWS $(SRC) -o $(BUILD_WIN) -Ofast

