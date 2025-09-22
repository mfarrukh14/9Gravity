#!/bin/bash
# Build script for Linux/macOS - 9Gravity Game Engine
# SDL3 libraries are included in the project!

echo "9Gravity Game Engine - Unix Build Script"
echo "========================================"
echo "SDL3 libraries are included in the project!"
echo

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Parse command line arguments
BUILD_SYSTEM=""
case "$1" in
    cmake)
        BUILD_SYSTEM="cmake"
        ;;
    make)
        BUILD_SYSTEM="make"
        ;;
    help|--help|-h)
        echo "Usage: $0 [cmake|make|help]"
        echo
        echo "  cmake  - Force CMake build (recommended)"
        echo "  make   - Force Make build"
        echo "  help   - Show this help"
        echo
        echo "Default behavior: Try CMake first, fallback to Make"
        echo
        echo "Requirements:"
        echo "- GCC or Clang compiler"
        echo "- CMake (recommended) or Make"
        echo
        echo "SDL3 libraries are included in the project!"
        echo "No additional SDL3 installation required."
        exit 0
        ;;
esac

# Try CMake build first (unless make is explicitly requested)
if [[ "$BUILD_SYSTEM" != "make" ]]; then
    if command_exists cmake; then
        echo "Using CMake build system..."
        mkdir -p build
        cd build
        
        if cmake ..; then
            if make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4); then
                echo
                echo "✅ Build successful! Executable: build/9Gravity"
                echo "Run with: ./build/9Gravity"
                exit 0
            else
                echo "❌ CMake build failed, trying Make..."
                cd ..
            fi
        else
            echo "❌ CMake configuration failed, trying Make..."
            cd ..
        fi
    elif [[ "$BUILD_SYSTEM" == "cmake" ]]; then
        echo "❌ CMake not found but explicitly requested!"
        echo "Install CMake or use: $0 make"
        exit 1
    fi
fi

# Try Make build
if command_exists make; then
    echo "Using Make build system..."
    
    if make; then
        echo
        echo "✅ Build successful! Executable: 9Gravity"
        echo "Run with: ./9Gravity"
    else
        echo "❌ Make build failed!"
        echo "Please ensure you have GCC or Clang installed."
        echo "SDL3 libraries are included in the project."
        exit 1
    fi
else
    echo "❌ Neither CMake nor Make found!"
    echo "Please install either CMake or Make to build this project."
    exit 1
fi
