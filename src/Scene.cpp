#include "Scene.h"
#include "Engine.h"
#include <algorithm>

// Scene Implementation
Scene::Scene() : m_engine(nullptr) {
}

Scene::~Scene() {
    Cleanup();
}

void Scene::Update(float deltaTime) {
    for (auto& obj : m_gameObjects) {
        if (obj && obj->active) {
            obj->Update(deltaTime);
        }
    }
    
    // Remove inactive objects
    m_gameObjects.erase(
        std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
            [](const std::shared_ptr<GameObject>& obj) { return !obj || !obj->active; }),
        m_gameObjects.end()
    );
}

void Scene::Render(Renderer* renderer) {
    for (auto& obj : m_gameObjects) {
        if (obj && obj->active) {
            obj->Render(renderer);
        }
    }
}

void Scene::AddGameObject(std::shared_ptr<GameObject> obj) {
    if (obj) {
        obj->m_scene = this;
        m_gameObjects.push_back(obj);
    }
}

void Scene::RemoveGameObject(std::shared_ptr<GameObject> obj) {
    if (obj) {
        obj->active = false;
        obj->m_scene = nullptr;
    }
}

// GameObject Implementation
GameObject::GameObject() 
    : position(0, 0)
    , velocity(0, 0)
    , rotation(0)
    , scale(1, 1)
    , active(true)
    , m_scene(nullptr)
{
}

GameObject::~GameObject() {
}
