#include "../editor/gui/GameEditor.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <SDL3/SDL.h>            
#include <SDL3/SDL_opengl.h>
#ifdef HAVE_SDL3_IMAGE
#include <SDL3_image/SDL_image.h>
#endif
#include <stdio.h>
#include <filesystem>
#include <string>
#include <iostream>

const char* glsl_version = "#version 130";

enum class AppMode {
    Launcher,
    Editor,
    Switching
};

bool CreateSDLWindowAndContext(SDL_Window*& window, SDL_GLContext& gl_context, const char* title, int w, int h, bool fullscreen = false) {
    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (fullscreen) windowFlags |= SDL_WINDOW_FULLSCREEN;

    window = SDL_CreateWindow(title, w, h, windowFlags);
    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "Failed to create GL context: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    const char* gl_ver = (const char*)glGetString(GL_VERSION);
    std::cout << "OpenGL version: " << (gl_ver ? gl_ver : "<null>") << std::endl;
    SDL_GL_SetSwapInterval(1); // vsync
    return true;
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    std::cout << "Starting 9Gravity Game Engine..." << std::endl;
    std::cout << "Testing basic functionality..." << std::endl;

    GameEditor editor;
    std::cout << "GameEditor created successfully!" << std::endl;
    std::cout << "Build number: " << editor.CurrentBuildNumber() << std::endl;
    
    std::cout << "Basic test passed! Now initializing SDL3..." << std::endl;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Error: SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

#ifdef HAVE_SDL3_IMAGE
    // Initialize SDL_image
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        std::cerr << "Image loading will be disabled." << std::endl;
    } else {
        std::cout << "SDL3 and SDL_image initialized successfully!" << std::endl;
    }
#else
    std::cout << "SDL3 initialized successfully! (Image loading disabled - SDL3_image not available)" << std::endl;
#endif

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!CreateSDLWindowAndContext(window, gl_context, "Game Editor Launcher", 1280, 720, false)) {
        return -1;
    }

    // Create a renderer for texture loading
    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Set the renderer for the editor
    editor.SetRenderer(renderer);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool running = true;
    AppMode mode = AppMode::Launcher;

    bool requestOpenFileDialog = false;
    std::string loadedProjectPath;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                running = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        for (ImTextureData* tex : ImGui::GetPlatformIO().Textures) {
            if (tex && tex->Status != ImTextureStatus_OK) {
                ImGui_ImplOpenGL3_UpdateTexture(tex);
            }
        }

        if (mode == AppMode::Launcher) {
            bool switchToEditor = editor.RenderLauncher(requestOpenFileDialog, loadedProjectPath);

            if (!loadedProjectPath.empty()) {
                mode = AppMode::Switching;
                // Don't continue here - we need to finish the frame
            }

            if (requestOpenFileDialog) {
                static char pathBuf[1024] = "";
                ImGui::OpenPopup("Load Project Path");
                if (ImGui::BeginPopupModal("Load Project Path", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::InputText("Path", pathBuf, sizeof(pathBuf));
                    if (ImGui::Button("Load")) {
                        if (std::filesystem::exists(pathBuf)) {
                            editor.OpenProject(pathBuf);
                            mode = AppMode::Switching;
                            requestOpenFileDialog = false;
                        } else {
                            std::cerr << "Path doesn't exist: " << pathBuf << std::endl;
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                        requestOpenFileDialog = false;
                    }
                    ImGui::EndPopup();
                }
            }
        } else if (mode == AppMode::Switching) {
            // Handle the mode switch here, after ImGui has finished rendering the previous frame
            if (!loadedProjectPath.empty()) {
                // Finish current frame before switching
                ImGui::Render();
                glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
                glClearColor(0.1f, 0.12f, 0.12f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                SDL_GL_SwapWindow(window);
                
                // Switch to editor mode
                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplSDL3_Shutdown();
                SDL_DestroyRenderer(renderer);
                SDL_GL_DestroyContext(gl_context);
                SDL_DestroyWindow(window);

                if (!CreateSDLWindowAndContext(window, gl_context, ("Game Editor - " + loadedProjectPath).c_str(), 1280, 720, true)) {
                    std::cerr << "Failed to create fullscreen editor window" << std::endl;
                    running = false;
                    break;
                }

                // Create new renderer for the new window
                renderer = SDL_CreateRenderer(window, NULL);
                if (!renderer) {
                    std::cerr << "Failed to create renderer for editor: " << SDL_GetError() << std::endl;
                    running = false;
                    break;
                }
                editor.SetRenderer(renderer);

                ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
                ImGui_ImplOpenGL3_Init(glsl_version);

                editor.OpenProject(loadedProjectPath);
                mode = AppMode::Editor;
                loadedProjectPath.clear();
                continue; // Skip rendering this frame
            } else {
                mode = AppMode::Launcher;
            }
        } else { 
            editor.RenderEditor();
        }

        ImGui::Render();
        {
            static bool s_printed = false;
            if (!s_printed) {
                auto& textures = ImGui::GetPlatformIO().Textures;
                std::cout << "ImGui Textures count: " << textures.size() << std::endl;
                for (size_t i = 0; i < textures.size() && i < 4; ++i) {
                    ImTextureData* tex = textures[i];
                    if (tex) {
                        std::cout << "  tex[" << i << "]: status=" << (int)tex->Status << ", id=" << (void*)tex->TexID << ", wxh=" << tex->Width << "x" << tex->Height << std::endl;
                    }
                }
                s_printed = true;
            }
        }
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(0.1f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (renderer) SDL_DestroyRenderer(renderer);
    if (gl_context) SDL_GL_DestroyContext(gl_context);
    if (window) SDL_DestroyWindow(window);
#ifdef HAVE_SDL3_IMAGE
    IMG_Quit();
#endif
    SDL_Quit();

    return 0;
}
