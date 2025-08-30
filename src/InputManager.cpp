#include "InputManager.h"

InputManager::InputManager() : m_mouseX(0), m_mouseY(0) {
}

InputManager::~InputManager() {
}

void InputManager::Update() {
    // Update key states (convert PRESSED to DOWN, RELEASED to UP)
    for (auto& pair : m_keyStates) {
        if (pair.second == KeyState::PRESSED) {
            pair.second = KeyState::DOWN;
        } else if (pair.second == KeyState::RELEASED) {
            pair.second = KeyState::UP;
        }
    }
    
    // Update mouse states
    for (auto& pair : m_mouseStates) {
        if (pair.second == KeyState::PRESSED) {
            pair.second = KeyState::DOWN;
        } else if (pair.second == KeyState::RELEASED) {
            pair.second = KeyState::UP;
        }
    }
}

void InputManager::HandleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            if (m_keyStates[event.key.key] != KeyState::DOWN) {
                m_keyStates[event.key.key] = KeyState::PRESSED;
            }
            break;
            
        case SDL_EVENT_KEY_UP:
            m_keyStates[event.key.key] = KeyState::RELEASED;
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (m_mouseStates[event.button.button] != KeyState::DOWN) {
                m_mouseStates[event.button.button] = KeyState::PRESSED;
            }
            break;
            
        case SDL_EVENT_MOUSE_BUTTON_UP:
            m_mouseStates[event.button.button] = KeyState::RELEASED;
            break;
            
        case SDL_EVENT_MOUSE_MOTION:
            m_mouseX = (int)event.motion.x;
            m_mouseY = (int)event.motion.y;
            break;
    }
}

bool InputManager::IsKeyDown(SDL_Keycode key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && 
           (it->second == KeyState::DOWN || it->second == KeyState::PRESSED);
}

bool InputManager::IsKeyPressed(SDL_Keycode key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && it->second == KeyState::PRESSED;
}

bool InputManager::IsKeyReleased(SDL_Keycode key) const {
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && it->second == KeyState::RELEASED;
}

bool InputManager::IsMouseButtonDown(Uint8 button) const {
    auto it = m_mouseStates.find(button);
    return it != m_mouseStates.end() && 
           (it->second == KeyState::DOWN || it->second == KeyState::PRESSED);
}

bool InputManager::IsMouseButtonPressed(Uint8 button) const {
    auto it = m_mouseStates.find(button);
    return it != m_mouseStates.end() && it->second == KeyState::PRESSED;
}

bool InputManager::IsMouseButtonReleased(Uint8 button) const {
    auto it = m_mouseStates.find(button);
    return it != m_mouseStates.end() && it->second == KeyState::RELEASED;
}
