#include "AssetManager.h"
#include <iostream>

AssetManager::AssetManager() : m_renderer(nullptr) {
}

AssetManager::~AssetManager() {
    UnloadAllTextures();
}

std::shared_ptr<Texture> AssetManager::LoadTexture(const std::string& name, const std::string& path) {
    // Check if texture is already loaded
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    
    // Load new texture
    auto texture = std::make_shared<Texture>();
    if (texture->LoadFromFile(m_renderer->GetSDLRenderer(), path)) {
        m_textures[name] = texture;
        std::cout << "Loaded texture: " << name << " from " << path << std::endl;
        return texture;
    }
    
    std::cerr << "Failed to load texture: " << name << " from " << path << std::endl;
    return nullptr;
}

std::shared_ptr<Texture> AssetManager::GetTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    return nullptr;
}

void AssetManager::UnloadTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        m_textures.erase(it);
        std::cout << "Unloaded texture: " << name << std::endl;
    }
}

void AssetManager::UnloadAllTextures() {
    m_textures.clear();
    std::cout << "All textures unloaded" << std::endl;
}
