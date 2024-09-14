CC = g++

SRCS = $(wildcard src/*.cpp)
SRC ?= $(SRCS)
OBJS = $(foreach file, $(SRCS), build/$(notdir $(file:.cpp=.o)))
BUILD_LINUX = build/msl
BUILD_WIN = build/msl.exe

debug:
ifeq ($(OS),Windows_NT)
	$(foreach file, $(SRC), $(CC) -std=c++20 -D DEBUG -D WINDOWS -c $(file) -o build/$(notdir $(file:.cpp=.o)) -fsanitize=leak -g -O0;)
	g++ $(OBJS) -o $(BUILD_WIN)
else
	$(foreach file, $(SRC), $(CC) -std=c++20 -D DEBUG -D LINUX -c $(file) -o build/$(notdir $(file:.cpp=.o)) -fsanitize=leak -g -O0;)
	g++ $(OBJS) -o $(BUILD_LINUX)
endif

performance:
ifeq ($(OS),Windows_NT)
	$(foreach file, $(SRC), $(CC) -std=c++20 -D DEBUG -D WINDOWS -c $(file) -o bulid/$(notdir $(file:.cpp=.o)) -O3;)
	g++ $(OBJS) -o $(BUILD_WIN)
else
	$(foreach file, $(SRC), $(CC) -std=c++20 -D DEBUG -D LINUX -c $(file) -o build/$(notdir $(file:.cpp=.o)) -O3;)
	g++ $(OBJS) -o $(BUILD_LINUX)
endif

release:
ifeq ($(OS),Windows_NT)
	$(foreach file, $(SRC), $(CC) -std=c++20 -D WINDOWS -c $(file) -o build/$(notdir $(file:.cpp=.o)) -O3;)
	g++ $(OBJS) -o $(BUILD_WIN)
else
	$(foreach file, $(SRC), $(CC) -std=c++20 -D LINUX -c $(file) -o build/$(notdir $(file:.cpp=.o)) -O3;)
	g++ $(OBJS) -o $(BUILD_LINUX)
endif

clean:
	$(foreach file, $(OBJS), rm $(file);)
