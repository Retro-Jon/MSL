CC = g++
SRC = $(wildcard src/*.cpp)
BUILD_LINUX = build/msol
BUILD_WIN = build/msol.exe

debug:
ifeq ($(OS),Windows_NT)
	$(CC) -D WINDOWS -D DEBUG $(SRC) -o $(BUILD_WIN) -fsanitize=address -g -O0
else
	$(CC) -D LINUX -D DEBUG $(SRC) -o $(BUILD_LINUX) -fsanitize=address -g -O0
endif

performance:
ifeq ($(OS),Windows_NT)
	$(CC) -D WINDOWS -D DEBUG $(SRC) -o $(BUILD_WIN) -Ofast
else
	$(CC) -D LINUX -D DEBUG $(SRC) -o $(BUILD_LINUX) -Ofast
endif

release:
ifeq ($(OS),Windows_NT)
	$(CC) -D WINDOWS $(SRC) -o $(BUILD_WIN) -Ofast
else
	$(CC) -D LINUX $(SRC) -o $(BUILD_LINUX) -Ofast
endif

