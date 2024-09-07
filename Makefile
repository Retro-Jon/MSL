CC = g++
SRC = $(wildcard src/*.cpp)
BUILD_LINUX = build/msl
BUILD_WIN = build/msl.exe

debug:
ifeq ($(OS),Windows_NT)
	$(CC) -std=c++20 -D WINDOWS -D DEBUG $(SRC) -o $(BUILD_WIN) -fsanitize=leak -g -O0
else
	$(CC) -std=c++20 -D LINUX -D DEBUG $(SRC) -o $(BUILD_LINUX) -fsanitize=leak -g -O0
endif

performance:
ifeq ($(OS),Windows_NT)
	$(CC) -std=c++20 -D WINDOWS -D DEBUG $(SRC) -o $(BUILD_WIN) -O3
else
	$(CC) -std=c++20 -D LINUX -D DEBUG $(SRC) -o $(BUILD_LINUX) -O3
endif

release:
ifeq ($(OS),Windows_NT)
	$(CC) -std=c++20 -D WINDOWS $(SRC) -o $(BUILD_WIN) -O3
else
	$(CC) -std=c++20 -D LINUX $(SRC) -o $(BUILD_LINUX) -O3
endif

