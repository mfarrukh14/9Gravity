#pragma once

#include "Renderer.h"
#include <string>
#include <unordered_map>
#include <memory>

class AssetManager {
public:
    AssetManager();
    ~AssetManager();
    
    void SetRenderer(Renderer* renderer) { m_renderer = renderer; }
    
    std::shared_ptr<Texture> LoadTexture(const std::string& name, const std::string& path);
    std::shared_ptr<Texture> GetTexture(const std::string& name);
    
    void UnloadTexture(const std::string& name);
    void UnloadAllTextures();
    
private:
    Renderer* m_renderer;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};
