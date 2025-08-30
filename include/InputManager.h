#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

enum class KeyState {
    UP,
    DOWN,
    PRESSED,
    RELEASED
};

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    void Update();
    void HandleEvent(const SDL_Event& event);
    
    bool IsKeyDown(SDL_Keycode key) const;
    bool IsKeyPressed(SDL_Keycode key) const;
    bool IsKeyReleased(SDL_Keycode key) const;
    
    bool IsMouseButtonDown(Uint8 button) const;
    bool IsMouseButtonPressed(Uint8 button) const;
    bool IsMouseButtonReleased(Uint8 button) const;
    
    int GetMouseX() const { return m_mouseX; }
    int GetMouseY() const { return m_mouseY; }
    
private:
    std::unordered_map<SDL_Keycode, KeyState> m_keyStates;
    std::unordered_map<Uint8, KeyState> m_mouseStates;
    int m_mouseX, m_mouseY;
};
