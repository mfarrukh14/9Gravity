#pragma once

#include "Renderer.h"
#include <vector>
#include <memory>

// Forward declarations
class Engine;
class GameObject;

class Scene {
public:
    Scene();
    virtual ~Scene();
    
    virtual void Initialize() {}
    virtual void Update(float deltaTime);
    virtual void Render(Renderer* renderer);
    virtual void Cleanup() {}
    
    void AddGameObject(std::shared_ptr<GameObject> obj);
    void RemoveGameObject(std::shared_ptr<GameObject> obj);
    
    Engine* GetEngine() const { return m_engine; }
    void SetEngine(Engine* engine) { m_engine = engine; }
    
protected:
    std::vector<std::shared_ptr<GameObject>> m_gameObjects;
    Engine* m_engine;
};

class GameObject {
public:
    GameObject();
    virtual ~GameObject();
    
    virtual void Update(float deltaTime) {}
    virtual void Render(Renderer* renderer) {}
    
    Vector2 position;
    Vector2 velocity;
    float rotation;
    Vector2 scale;
    bool active;
    
protected:
    Scene* m_scene;
    friend class Scene;
};
