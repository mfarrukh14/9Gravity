#include "Engine.h"
#include "Renderer.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "AssetManager.h"
#include <iostream>

Engine::Engine() 
    : m_window(nullptr)
    , m_isRunning(false)
    , m_lastTime(0)
{
}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize(const std::string& title, int width, int height) {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create window
    m_window = SDL_CreateWindow(
        title.c_str(),
        width, height,
        SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Initialize subsystems
    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Initialize(m_window)) {
        std::cerr << "Renderer failed to initialize!" << std::endl;
        return false;
    }

    m_audioManager = std::make_unique<AudioManager>();
    if (!m_audioManager->Initialize()) {
        std::cerr << "Audio Manager failed to initialize!" << std::endl;
        return false;
    }

    m_inputManager = std::make_unique<InputManager>();
    
    m_assetManager = std::make_unique<AssetManager>();
    m_assetManager->SetRenderer(m_renderer.get());

    m_isRunning = true;
    m_lastTime = SDL_GetTicksNS();
    
    std::cout << "Engine initialized successfully!" << std::endl;
    return true;
}

void Engine::Run() {
    while (m_isRunning) {
        Uint64 currentTime = SDL_GetTicksNS();
        float deltaTime = (currentTime - m_lastTime) / 1000000000.0f; // Convert to seconds
        m_lastTime = currentTime;

        HandleEvents();
        Update(deltaTime);
        Render();
    }
}

void Engine::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_isRunning = false;
        }
        
        m_inputManager->HandleEvent(event);
    }
    
    m_inputManager->Update();
}

void Engine::Update(float deltaTime) {
    // Override in derived classes or use scene system
}

void Engine::Render() {
    m_renderer->Clear();
    
    // Override in derived classes or use scene system
    
    m_renderer->Present();
}

void Engine::Shutdown() {
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    
    m_assetManager.reset();
    m_audioManager.reset();
    m_inputManager.reset();
    m_renderer.reset();
    
    SDL_Quit();
    m_isRunning = false;
}
