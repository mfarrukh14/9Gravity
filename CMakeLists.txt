cmake_minimum_required(VERSION 3.16)
project(9Gravity VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Detect platform and architecture
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL_ARCH "x86_64-w64-mingw32")
    else()
        set(SDL_ARCH "i686-w64-mingw32")
    endif()
    set(SDL3_ROOT_DIR "${CMAKE_SOURCE_DIR}/SDL/${SDL_ARCH}")
    set(SDL3_INCLUDE_DIR "${SDL3_ROOT_DIR}/include")
    set(SDL3_LIBRARY_DIR "${SDL3_ROOT_DIR}/lib")
else()
    # On Linux/Unix, we don't have local SDL3 in the project
    # Skip local SDL3 detection and go straight to system detection
    set(SDL3_ROOT_DIR "")
    set(SDL3_INCLUDE_DIR "")
    set(SDL3_LIBRARY_DIR "")
endif()

# Find SDL3 library
if(WIN32)
    find_library(SDL3_LIBRARY
        NAMES SDL3 libSDL3
        PATHS ${SDL3_LIBRARY_DIR}
        NO_DEFAULT_PATH
    )

    # Check if we found SDL3 locally
    if(SDL3_LIBRARY AND EXISTS "${SDL3_INCLUDE_DIR}/SDL3/SDL.h")
        set(SDL3_FOUND TRUE)
        message(STATUS "Found local SDL3: ${SDL3_LIBRARY}")
        
        # Create imported target
        add_library(SDL3::SDL3 UNKNOWN IMPORTED)
        set_target_properties(SDL3::SDL3 PROPERTIES
            IMPORTED_LOCATION "${SDL3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIR}"
        )
    else()
        set(SDL3_FOUND FALSE)
    endif()
else()
    # On Linux/Unix, skip local SDL3 and go straight to system detection
    set(SDL3_FOUND FALSE)
endif()

if(NOT SDL3_FOUND)
    # Fallback to system SDL3
    message(STATUS "Local SDL3 not found, trying system installation...")
    find_package(SDL3 QUIET CONFIG)
    if(NOT SDL3_FOUND)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(SDL3 QUIET sdl3)
        endif()
    endif()
    
    # Manual fallback
    if(NOT SDL3_FOUND)
        find_path(SDL3_INCLUDE_DIR SDL3/SDL.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/homebrew/include
                C:/SDL3/include
        )
        
        find_library(SDL3_LIBRARY
            NAMES SDL3 libSDL3
            PATHS
                /usr/lib
                /usr/local/lib
                /opt/homebrew/lib
                C:/SDL3/lib
        )
        
        if(SDL3_INCLUDE_DIR AND SDL3_LIBRARY)
            set(SDL3_FOUND TRUE)
            add_library(SDL3::SDL3 UNKNOWN IMPORTED)
            set_target_properties(SDL3::SDL3 PROPERTIES
                IMPORTED_LOCATION "${SDL3_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIR}"
            )
        endif()
    endif()
endif()

if(NOT SDL3_FOUND)
    message(FATAL_ERROR "SDL3 not found! Please ensure SDL3 is in the SDL/ directory or install it system-wide")
endif()

# Find SDL3_image
find_package(SDL3_image QUIET CONFIG)
if(NOT SDL3_image_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL3_image QUIET SDL3_image)
    endif()
    
    # Manual fallback for SDL3_image
    if(NOT SDL3_image_FOUND)
        find_path(SDL3_image_INCLUDE_DIR SDL3_image/SDL_image.h
            PATHS
                /usr/include
                /usr/local/include
                /opt/homebrew/include
        )
        
        find_library(SDL3_image_LIBRARY
            NAMES SDL3_image libSDL3_image
            PATHS
                /usr/lib
                /usr/local/lib
                /opt/homebrew/lib
        )
        
        if(SDL3_image_INCLUDE_DIR AND SDL3_image_LIBRARY)
            set(SDL3_image_FOUND TRUE)
            add_library(SDL3_image::SDL3_image UNKNOWN IMPORTED)
            set_target_properties(SDL3_image::SDL3_image PROPERTIES
                IMPORTED_LOCATION "${SDL3_image_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${SDL3_image_INCLUDE_DIR}"
            )
            message(STATUS "Found SDL3_image: ${SDL3_image_LIBRARY}")
        else()
            message(WARNING "SDL3_image not found! Image loading will be disabled.")
        endif()
    endif()
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/Engine.cpp
    src/Renderer.cpp
    src/InputManager.cpp
    src/AudioManager.cpp
    src/AssetManager.cpp
    src/Scene.cpp
    src/Physics.cpp
    editor/gui/GameEditor.cpp
    # ImGui sources
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    # ImGui backends
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE imgui)
target_include_directories(${PROJECT_NAME} PRIVATE imgui/backends)

# Link SDL3
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Link OpenGL
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

# Link optional libraries if found
if(SDL3_image_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_image::SDL3_image)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SDL3_IMAGE)
endif()

if(SDL3_mixer_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_mixer::SDL3_mixer)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SDL3_MIXER)
endif()

# Platform-specific settings
if(WIN32)
    # Copy SDL3 DLL from local directory
    set(SDL3_DLL_PATH "${SDL3_ROOT_DIR}/bin/SDL3.dll")
    if(EXISTS "${SDL3_DLL_PATH}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL3_DLL_PATH}"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying SDL3.dll from local directory"
        )
    else()
        # Fallback: try to copy from project root
        if(EXISTS "${CMAKE_SOURCE_DIR}/SDL3.dll")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/SDL3.dll"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMENT "Copying SDL3.dll from project root"
            )
        endif()
    endif()
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Copy assets directory to build directory
if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying assets"
    )
endif()
