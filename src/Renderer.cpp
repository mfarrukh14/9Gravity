#include "Renderer.h"
#include <iostream>

// Texture Implementation
Texture::Texture() : m_texture(nullptr), m_width(0), m_height(0) {
}

Texture::~Texture() {
    Free();
}

bool Texture::LoadFromFile(SDL_Renderer* renderer, const std::string& path) {
    Free();
    
    // For now, create a simple colored texture instead of loading from file
    // This is because SDL3_image is not available
    m_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 32, 32);
    if (!m_texture) {
        std::cerr << "Unable to create texture! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Set as render target and fill with color
    SDL_SetRenderTarget(renderer, m_texture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, nullptr);
    
    m_width = 32;
    m_height = 32;
    
    std::cout << "Created placeholder texture for: " << path << std::endl;
    return true;
}

void Texture::Free() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
        m_width = 0;
        m_height = 0;
    }
}

void Texture::Render(SDL_Renderer* renderer, int x, int y, SDL_Rect* clip) {
    SDL_FRect renderQuad = { (float)x, (float)y, (float)m_width, (float)m_height };
    
    if (clip) {
        renderQuad.w = (float)clip->w;
        renderQuad.h = (float)clip->h;
    }
    
    SDL_FRect* srcRect = nullptr;
    SDL_FRect src;
    if (clip) {
        src = { (float)clip->x, (float)clip->y, (float)clip->w, (float)clip->h };
        srcRect = &src;
    }
    
    SDL_RenderTexture(renderer, m_texture, srcRect, &renderQuad);
}

void Texture::Render(SDL_Renderer* renderer, const Rect& destRect, SDL_Rect* clip) {
    SDL_FRect renderQuad = { destRect.x, destRect.y, destRect.width, destRect.height };
    
    SDL_FRect* srcRect = nullptr;
    SDL_FRect src;
    if (clip) {
        src = { (float)clip->x, (float)clip->y, (float)clip->w, (float)clip->h };
        srcRect = &src;
    }
    
    SDL_RenderTexture(renderer, m_texture, srcRect, &renderQuad);
}

// Renderer Implementation
Renderer::Renderer() : m_renderer(nullptr) {
}

Renderer::~Renderer() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
}

bool Renderer::Initialize(SDL_Window* window) {
    m_renderer = SDL_CreateRenderer(window, nullptr);
    if (!m_renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    return true;
}

void Renderer::Clear(const Color& color) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_renderer);
}

void Renderer::Present() {
    SDL_RenderPresent(m_renderer);
}

void Renderer::DrawRect(const Rect& rect, const Color& color, bool filled) {
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    
    SDL_FRect sdlRect = { rect.x, rect.y, rect.width, rect.height };
    
    if (filled) {
        SDL_RenderFillRect(m_renderer, &sdlRect);
    } else {
        SDL_RenderRect(m_renderer, &sdlRect);
    }
}

void Renderer::DrawTexture(Texture* texture, const Vector2& position, const Rect* sourceRect) {
    if (!texture) return;
    
    SDL_Rect* srcRect = nullptr;
    SDL_Rect src;
    if (sourceRect) {
        src = { (int)sourceRect->x, (int)sourceRect->y, (int)sourceRect->width, (int)sourceRect->height };
        srcRect = &src;
    }
    
    texture->Render(m_renderer, (int)position.x, (int)position.y, srcRect);
}

void Renderer::DrawTexture(Texture* texture, const Rect& destRect, const Rect* sourceRect) {
    if (!texture) return;
    
    SDL_Rect* srcRect = nullptr;
    SDL_Rect src;
    if (sourceRect) {
        src = { (int)sourceRect->x, (int)sourceRect->y, (int)sourceRect->width, (int)sourceRect->height };
        srcRect = &src;
    }
    
    texture->Render(m_renderer, destRect, srcRect);
}
