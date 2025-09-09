#include "../editor/gui/GameEditor.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <SDL3/SDL.h>            
#include <SDL3/SDL_opengl.h>     
#include <stdio.h>
#include <filesystem>
#include <string>
#include <iostream>

const char* glsl_version = "#version 130";

enum class AppMode {
    Launcher,
    Editor
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

    std::cout << "SDL3 initialized successfully!" << std::endl;

    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;

    if (!CreateSDLWindowAndContext(window, gl_context, "Game Editor Launcher", 1280, 720, false)) {
        return -1;
    }

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

        if (mode == AppMode::Launcher) {
            bool switchToEditor = editor.RenderLauncher(requestOpenFileDialog, loadedProjectPath);

            if (!loadedProjectPath.empty()) {

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                SDL_GL_SwapWindow(window);

                ImGui_ImplOpenGL3_Shutdown();
                ImGui_ImplSDL3_Shutdown();
                SDL_GL_DestroyContext(gl_context);
                SDL_DestroyWindow(window);

                if (!CreateSDLWindowAndContext(window, gl_context, ("Game Editor - " + loadedProjectPath).c_str(), 1280, 720, true)) {
                    std::cerr << "Failed to create fullscreen editor window" << std::endl;
                    running = false;
                    break;
                }

                ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
                ImGui_ImplOpenGL3_Init(glsl_version);

                editor.OpenProject(loadedProjectPath);
                mode = AppMode::Editor;
                loadedProjectPath.clear();
            }

            if (requestOpenFileDialog) {
                static char pathBuf[1024] = "";
                ImGui::OpenPopup("Load Project Path");
                if (ImGui::BeginPopupModal("Load Project Path", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::InputText("Path", pathBuf, sizeof(pathBuf));
                    if (ImGui::Button("Load")) {
                        if (std::filesystem::exists(pathBuf)) {
                            editor.OpenProject(pathBuf);

                            ImGui::Render();
                            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                            SDL_GL_SwapWindow(window);

                            ImGui_ImplOpenGL3_Shutdown();
                            ImGui_ImplSDL3_Shutdown();
                            SDL_GL_DestroyContext(gl_context);
                            SDL_DestroyWindow(window);

                            if (!CreateSDLWindowAndContext(window, gl_context, ("Game Editor - " + std::string(pathBuf)).c_str(), 1280, 720, true)) {
                                std::cerr << "Failed to create fullscreen editor window" << std::endl;
                                running = false;
                                break;
                            }
                            ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
                            ImGui_ImplOpenGL3_Init(glsl_version);

                            mode = AppMode::Editor;
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
        } else { 
            editor.RenderEditor();
        }

        ImGui::Render();
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(0.1f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (gl_context) SDL_GL_DestroyContext(gl_context);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
