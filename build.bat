@echo off
REM Build script for Windows - 9Gravity Game Engine
echo 9Gravity Game Engine - Windows Build Script
echo =============================================
echo SDL3 libraries are included in the project!
echo.

REM Check if we have a preference for build system
if "%1"=="cmake" goto cmake_build
if "%1"=="make" goto make_build
if "%1"=="help" goto show_help

REM Default: try CMake first, fallback to Make, then direct compilation
echo Attempting CMake build...
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo CMake not found, trying Make...
    goto make_build
)

:cmake_build
echo Using CMake build system...
if not exist build mkdir build
cd build
cmake .. -G "MinGW Makefiles"
if %errorlevel% neq 0 (
    echo CMake configuration failed, trying Make...
    cd ..
    goto make_build
)
cmake --build .
if %errorlevel% equ 0 (
    echo Build successful! Executable: build\9Gravity.exe
    echo Run with: build\9Gravity.exe
) else (
    echo CMake build failed, trying Make...
    cd ..
    goto make_build
)
goto end

:make_build
echo Using Make build system...
make --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Make not found! Trying direct compilation...
    goto direct_compile
)
make
if %errorlevel% equ 0 (
    echo Build successful! Executable: 9Gravity.exe
    echo Run with: .\9Gravity.exe
) else (
    echo Make build failed! Trying direct compilation...
    goto direct_compile
)
goto end

:direct_compile
echo Using direct GCC compilation...
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo GCC not found! Please install MinGW-w64.
    goto error_end
)

echo Compiling with GCC...
g++ -std=c++17 -Iinclude -I"SDL/x86_64-w64-mingw32/include" -Iimgui -Iimgui/backends editor/gui/GameEditor.cpp src/main.cpp src/Engine.cpp src/Renderer.cpp src/InputManager.cpp src/AudioManager.cpp src/AssetManager.cpp src/Scene.cpp src/Physics.cpp imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp imgui/backends/imgui_impl_sdl3.cpp imgui/backends/imgui_impl_opengl3.cpp -o 9Gravity.exe -L"SDL/x86_64-w64-mingw32/lib" -lSDL3 -lopengl32

if %errorlevel% equ 0 (
    echo Copying SDL3.dll...
    copy "SDL\x86_64-w64-mingw32\bin\SDL3.dll" . >nul 2>&1
    echo Build successful! Executable: 9Gravity.exe
    echo Run with: .\9Gravity.exe
) else (
    echo Direct compilation failed!
    echo Please ensure you have GCC/MinGW-w64 installed.
    goto error_end
)
goto end

:show_help
echo Usage: build.bat [cmake^|make^|help]
echo.
echo   cmake  - Force CMake build (recommended)
echo   make   - Force Make build
echo   help   - Show this help
echo.
echo Default behavior: Try CMake first, fallback to Make, then direct compilation
echo.
echo Requirements:
echo - MinGW-w64 or Visual Studio compiler
echo - CMake (recommended) or Make
echo.
echo SDL3 is included in the project directory!
echo No additional SDL3 installation required.
goto end

:error_end
echo.
echo Build failed! For help run: build.bat help
exit /b 1

:end
