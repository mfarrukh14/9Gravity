#include "Engine.h"
#include "Scene.h"
#include "Physics.h"
#include "InputManager.h"
#include <iostream>
#include <memory>

class Ball : public GameObject {
public:
    Ball(float x, float y) : size(20.0f) {
        position = Vector2(x, y);
        velocity = Vector2(100.0f, -200.0f); // Initial velocity
        
        // Physics body
        body.position = position;
        body.velocity = velocity;
        body.mass = 1.0f;
        body.restitution = 0.8f;
    }
    
    void Update(float deltaTime) override {
        // Apply gravity
        Physics::ApplyGravity(body, Vector2(0, 500.0f));
        
        // Update physics
        Physics::UpdateBody(body, deltaTime);
        
        // Update game object position
        position = body.position;
        velocity = body.velocity;
        
        // Simple boundary collision (screen wrap/bounce)
        if (position.x - size/2 < 0) {
            position.x = size/2;
            body.position.x = position.x;
            body.velocity.x = -body.velocity.x * body.restitution;
        }
        if (position.x + size/2 > 800) {
            position.x = 800 - size/2;
            body.position.x = position.x;
            body.velocity.x = -body.velocity.x * body.restitution;
        }
        if (position.y - size/2 < 0) {
            position.y = size/2;
            body.position.y = position.y;
            body.velocity.y = -body.velocity.y * body.restitution;
        }
        if (position.y + size/2 > 600) {
            position.y = 600 - size/2;
            body.position.y = position.y;
            body.velocity.y = -body.velocity.y * body.restitution;
        }
    }
    
    void Render(Renderer* renderer) override {
        Rect rect(position.x - size/2, position.y - size/2, size, size);
        renderer->DrawRect(rect, Color(255, 100, 100, 255));
    }
    
private:
    float size;
    Physics::Body body;
};

class GameScene : public Scene {
public:
    void Initialize() override {
        // Add some bouncing balls
        AddGameObject(std::make_shared<Ball>(100, 100));
        AddGameObject(std::make_shared<Ball>(200, 150));
        AddGameObject(std::make_shared<Ball>(300, 100));
        
        std::cout << "Game scene initialized with bouncing balls!" << std::endl;
    }
    
    void Update(float deltaTime) override {
        Scene::Update(deltaTime);
        
        // Example: Add new ball on space press
        if (GetEngine() && GetEngine()->GetInputManager()->IsKeyPressed(SDLK_SPACE)) {
            AddGameObject(std::make_shared<Ball>(400, 100));
        }
        
        // Quit on ESC
        if (GetEngine() && GetEngine()->GetInputManager()->IsKeyPressed(SDLK_ESCAPE)) {
            GetEngine()->Quit();
        }
    }
};

class GameEngine : public Engine {
public:
    bool Initialize() {
        if (!Engine::Initialize("Simple 2D Game Engine - Demo", 800, 600)) {
            return false;
        }
        
        // Create and initialize scene
        m_scene = std::make_unique<GameScene>();
        m_scene->SetEngine(this);
        m_scene->Initialize();
        
        return true;
    }
    
protected:
    virtual void Update(float deltaTime) {
        if (m_scene) {
            m_scene->Update(deltaTime);
        }
    }
    
    virtual void Render() {
        GetRenderer()->Clear(Color(50, 50, 100, 255)); // Dark blue background
        
        if (m_scene) {
            m_scene->Render(GetRenderer());
        }
        
        GetRenderer()->Present();
    }
    
private:
    std::unique_ptr<GameScene> m_scene;
};

int main(int argc, char* argv[]) {
    GameEngine engine;
    
    if (!engine.Initialize()) {
        std::cerr << "Failed to initialize engine!" << std::endl;
        return -1;
    }
    
    std::cout << "=== Simple 2D Game Engine Demo ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE - Add new ball" << std::endl;
    std::cout << "  ESC   - Quit" << std::endl;
    std::cout << "=================================" << std::endl;
    
    engine.Run();
    
    return 0;
}
