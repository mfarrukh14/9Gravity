#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>

class Renderer;
class AudioManager;
class InputManager;
class AssetManager;

class Engine {
public:
    Engine();
    ~Engine();

    bool Initialize(const std::string& title, int width, int height);
    void Run();
    void Shutdown();

    // Getters
    Renderer* GetRenderer() const { return m_renderer.get(); }
    AudioManager* GetAudioManager() const { return m_audioManager.get(); }
    InputManager* GetInputManager() const { return m_inputManager.get(); }
    AssetManager* GetAssetManager() const { return m_assetManager.get(); }
    
    bool IsRunning() const { return m_isRunning; }
    void Quit() { m_isRunning = false; }

private:
    void HandleEvents();
    virtual void Update(float deltaTime);
    virtual void Render();

    SDL_Window* m_window;
    bool m_isRunning;
    
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<AudioManager> m_audioManager;
    std::unique_ptr<InputManager> m_inputManager;
    std::unique_ptr<AssetManager> m_assetManager;
    
    Uint64 m_lastTime;
};
