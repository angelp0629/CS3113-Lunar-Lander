# Cat-astrophic Landing - follows the structure in Prof. Romero Cruz's
# resources/SET_UP.md, extended to compile Entity.cpp.
# IMPORTANT: recipe commands below are TAB-indented, not spaces.

SRCS = main.cpp CS3113/Entity.cpp
TARGET = cat_lander
UNAME_S := $(shell uname -s)

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS = $(shell pkg-config --libs raylib 2>/dev/null)

ifeq ($(UNAME_S),Darwin)
    CXXFLAGS += $(RAYLIB_CFLAGS)
    LIBS = $(RAYLIB_LIBS) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXEC = ./$(TARGET)
else ifneq (,$(findstring MINGW,$(UNAME_S)))
    CXXFLAGS += -IC:/raylib/include
    LIBS = -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET := $(TARGET).exe
    EXEC = ./$(TARGET)
else ifneq (,$(findstring MSYS,$(UNAME_S)))
    # Running a MinGW compiler from Git Bash/MSYS on Windows.
    CXXFLAGS += -IC:/raylib/include
    LIBS = -LC:/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET := $(TARGET).exe
    EXEC = ./$(TARGET)
else
    CXXFLAGS += $(RAYLIB_CFLAGS)
    LIBS = $(RAYLIB_LIBS)
    EXEC = ./$(TARGET)
endif

.PHONY: all run clean
all: $(TARGET)

$(TARGET): $(SRCS) CS3113/Entity.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

run: $(TARGET)
	$(EXEC)

clean:
	rm -f cat_lander cat_lander.exe
