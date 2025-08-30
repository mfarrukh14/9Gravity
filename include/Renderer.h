#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <memory>

struct Vector2 {
    float x, y;
    Vector2(float x = 0, float y = 0) : x(x), y(y) {}
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
};

struct Color {
    Uint8 r, g, b, a;
    Color(Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255) : r(r), g(g), b(b), a(a) {}
};

struct Rect {
    float x, y, width, height;
    Rect(float x = 0, float y = 0, float w = 0, float h = 0) : x(x), y(y), width(w), height(h) {}
};

class Texture {
public:
    Texture();
    ~Texture();
    
    bool LoadFromFile(SDL_Renderer* renderer, const std::string& path);
    void Free();
    
    void Render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip = nullptr);
    void Render(SDL_Renderer* renderer, const Rect& destRect, SDL_Rect* clip = nullptr);
    
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    
private:
    SDL_Texture* m_texture;
    int m_width;
    int m_height;
};

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    bool Initialize(SDL_Window* window);
    void Clear(const Color& color = Color(0, 0, 0, 255));
    void Present();
    
    void DrawRect(const Rect& rect, const Color& color, bool filled = true);
    void DrawTexture(Texture* texture, const Vector2& position, const Rect* sourceRect = nullptr);
    void DrawTexture(Texture* texture, const Rect& destRect, const Rect* sourceRect = nullptr);
    
    SDL_Renderer* GetSDLRenderer() const { return m_renderer; }
    
private:
    SDL_Renderer* m_renderer;
};
