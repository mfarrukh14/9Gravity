# Cross-platform Makefile for 9Gravity Game Engine
# Automatically detects platform and configures SDL3 paths

# Detect operating system
ifeq ($(OS),Windows_NT)
    PLATFORM := Windows
    EXECUTABLE_EXT := .exe
    PATH_SEP := \\
    COPY_CMD := copy
    RM_CMD := del /Q
    MKDIR_CMD := mkdir
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        PLATFORM := Linux
    endif
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := macOS
    endif
    EXECUTABLE_EXT := 
    PATH_SEP := /
    COPY_CMD := cp
    RM_CMD := rm -f
    MKDIR_CMD := mkdir -p
endif

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
INCLUDES := -Iinclude

# Source and object files
SRCDIR := src
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := 9Gravity$(EXECUTABLE_EXT)

# Detect architecture for local SDL
ifeq ($(PLATFORM),Windows)
    # Check processor architecture
    ifdef PROCESSOR_ARCHITEW6432
        ARCH := x86_64
    else
        ifdef PROCESSOR_ARCHITECTURE
            ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
                ARCH := x86_64
            else
                ARCH := i686
            endif
        else
            ARCH := x86_64
        endif
    endif
    SDL_ARCH := $(ARCH)-w64-mingw32
else
    ARCH := $(shell uname -m)
    ifeq ($(ARCH),x86_64)
        SDL_ARCH := x86_64
    else ifeq ($(ARCH),i686)
        SDL_ARCH := i686
    else
        SDL_ARCH := $(ARCH)
    endif
endif

# Platform-specific SDL3 configuration
ifeq ($(PLATFORM),Windows)
    # Use local SDL directory first
    LOCAL_SDL_PATH := ./SDL/$(SDL_ARCH)
    ifneq ($(wildcard $(LOCAL_SDL_PATH)),)
        INCLUDES += -I"$(LOCAL_SDL_PATH)/include"
        LIBS += -L"$(LOCAL_SDL_PATH)/lib" -lSDL3
        SDL_SOURCE := local
    else
        # Fallback to system paths
        SDL_PATHS := \
            "C:/SDL3" \
            "C:/libs/SDL3" \
            "C:/dev/SDL3" \
            "./libs/SDL3" \
            "$(USERPROFILE)/SDL3"
        
        SDL_PATH := $(firstword $(foreach path,$(SDL_PATHS),$(wildcard $(path))))
        
        ifneq ($(SDL_PATH),)
            INCLUDES += -I"$(SDL_PATH)/include"
            LIBS += -L"$(SDL_PATH)/lib" -lSDL3
            SDL_SOURCE := system
        else
            LIBS += -lSDL3
            SDL_SOURCE := default
        endif
    endif
    
    # Windows-specific flags
    LDFLAGS += -static-libgcc -static-libstdc++
    
else ifeq ($(PLATFORM),Linux)
    # Check for local SDL first (for consistent builds)
    LOCAL_SDL_PATH := ./SDL/linux-$(SDL_ARCH)
    ifneq ($(wildcard $(LOCAL_SDL_PATH)),)
        INCLUDES += -I"$(LOCAL_SDL_PATH)/include"
        LIBS += -L"$(LOCAL_SDL_PATH)/lib" -lSDL3
        SDL_SOURCE := local
    else
        # Try pkg-config first, then fallback to system paths
        SDL_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null)
        SDL_LIBS := $(shell pkg-config --libs sdl3 2>/dev/null)
        
        ifneq ($(SDL_CFLAGS),)
            INCLUDES += $(SDL_CFLAGS)
            LIBS += $(SDL_LIBS)
            SDL_SOURCE := pkg-config
        else
            # Fallback to common system paths
            INCLUDES += -I/usr/include/SDL3 -I/usr/local/include/SDL3
            LIBS += -lSDL3
            SDL_SOURCE := system
        endif
    endif
    
else ifeq ($(PLATFORM),macOS)
    # Check for local SDL first
    LOCAL_SDL_PATH := ./SDL/macos-$(SDL_ARCH)
    ifneq ($(wildcard $(LOCAL_SDL_PATH)),)
        INCLUDES += -I"$(LOCAL_SDL_PATH)/include"
        LIBS += -L"$(LOCAL_SDL_PATH)/lib" -lSDL3
        SDL_SOURCE := local
    else
        # Try pkg-config first, then Homebrew paths
        SDL_CFLAGS := $(shell pkg-config --cflags sdl3 2>/dev/null)
        SDL_LIBS := $(shell pkg-config --libs sdl3 2>/dev/null)
        
        ifneq ($(SDL_CFLAGS),)
            INCLUDES += $(SDL_CFLAGS)
            LIBS += $(SDL_LIBS)
            SDL_SOURCE := pkg-config
        else
            # Homebrew paths
            INCLUDES += -I/opt/homebrew/include/SDL3 -I/usr/local/include/SDL3
            LIBS += -L/opt/homebrew/lib -L/usr/local/lib -lSDL3
            SDL_SOURCE := homebrew
        endif
    endif
endif

# Targets
.PHONY: all clean run install help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $@ $(LIBS) $(LDFLAGS)
	@echo "Build complete!"

%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "Cleaning build files..."
	$(RM_CMD) $(OBJECTS) $(TARGET)
	@echo "Clean complete!"

run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Help target
help:
	@echo "9Gravity Game Engine - Build System"
	@echo "Available targets:"
	@echo "  all     - Build the game engine (default)"
	@echo "  clean   - Remove build files"
	@echo "  run     - Build and run the game"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Platform detected: $(PLATFORM)"
	@echo "Architecture: $(ARCH)"
	@echo "SDL3 source: $(SDL_SOURCE)"
	@echo ""
	@echo "SDL3 is included in the project directory!"
	@echo "No additional SDL3 installation required."
	@echo ""
	@echo "If build still fails, ensure you have:"
	@echo "  Windows: MinGW-w64 or Visual Studio"
	@echo "  Linux:   GCC or Clang"
	@echo "  macOS:   Xcode Command Line Tools"

# Dependencies
$(SRCDIR)/main.o: include/Engine.h include/Scene.h include/Physics.h
$(SRCDIR)/Engine.o: include/Engine.h include/Renderer.h include/AudioManager.h include/InputManager.h include/AssetManager.h
$(SRCDIR)/Renderer.o: include/Renderer.h
$(SRCDIR)/InputManager.o: include/InputManager.h
$(SRCDIR)/AudioManager.o: include/AudioManager.h
$(SRCDIR)/AssetManager.o: include/AssetManager.h include/Renderer.h
$(SRCDIR)/Scene.o: include/Scene.h include/Engine.h
$(SRCDIR)/Physics.o: include/Physics.h include/Renderer.h
