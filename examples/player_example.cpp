// Example: Simple Player Class
// This demonstrates how to create a controllable player using the engine

#include "Engine.h"
#include "Scene.h"
#include "Physics.h"
#include "InputManager.h"
#include <iostream>
#include <cmath>

class Player : public GameObject {
public:
    Player(float x, float y) : size(30.0f), speed(300.0f) {
        position = Vector2(x, y);
        
        // Physics setup
        body.position = position;
        body.mass = 1.0f;
        body.restitution = 0.1f;
        body.isStatic = false;
    }
    
    void Update(float deltaTime) override {
        auto* engine = m_scene->GetEngine();
        auto* input = engine->GetInputManager();
        
        // Movement input
        Vector2 acceleration(0, 0);
        if (input->IsKeyDown(SDLK_A) || input->IsKeyDown(SDLK_LEFT)) {
            acceleration.x -= speed;
        }
        if (input->IsKeyDown(SDLK_D) || input->IsKeyDown(SDLK_RIGHT)) {
            acceleration.x += speed;
        }
        if (input->IsKeyDown(SDLK_W) || input->IsKeyDown(SDLK_UP)) {
            acceleration.y -= speed;
        }
        if (input->IsKeyDown(SDLK_S) || input->IsKeyDown(SDLK_DOWN)) {
            acceleration.y += speed;
        }
        
        // Apply acceleration
        body.acceleration = acceleration;
        
        // Update physics
        Physics::UpdateBody(body, deltaTime);
        
        // Update position
        position = body.position;
        
        // Keep player on screen (800x600)
        if (position.x < size/2) {
            position.x = size/2;
            body.position.x = position.x;
            body.velocity.x = 0;
        }
        if (position.x > 800 - size/2) {
            position.x = 800 - size/2;
            body.position.x = position.x;
            body.velocity.x = 0;
        }
        if (position.y < size/2) {
            position.y = size/2;
            body.position.y = position.y;
            body.velocity.y = 0;
        }
        if (position.y > 600 - size/2) {
            position.y = 600 - size/2;
            body.position.y = position.y;
            body.velocity.y = 0;
        }
    }
    
    void Render(Renderer* renderer) override {
        // Draw player as a green rectangle
        Rect rect(position.x - size/2, position.y - size/2, size, size);
        renderer->DrawRect(rect, Color(100, 255, 100, 255));
    }
    
private:
    float size;
    float speed;
    Physics::Body body;
};

class Collectible : public GameObject {
public:
    Collectible(float x, float y) : size(15.0f), collected(false) {
        position = Vector2(x, y);
    }
    
    void Update(float deltaTime) override {
        // Simple spinning animation
        rotation += 180.0f * deltaTime; // 180 degrees per second
        
        if (rotation > 360.0f) {
            rotation -= 360.0f;
        }
    }
    
    void Render(Renderer* renderer) override {
        if (!collected) {
            // Draw collectible as a yellow diamond (approximated with rectangle)
            Rect rect(position.x - size/2, position.y - size/2, size, size);
            renderer->DrawRect(rect, Color(255, 255, 100, 255));
        }
    }
    
    bool CheckCollision(const Vector2& playerPos, float playerSize) {
        if (collected) return false;
        
        float dx = position.x - playerPos.x;
        float dy = position.y - playerPos.y;
        float distance = sqrt(dx*dx + dy*dy);
        
        if (distance < (size + playerSize) / 2) {
            collected = true;
            return true;
        }
        return false;
    }
    
private:
    float size;
    bool collected;
};

class GameScene : public Scene {
public:
    GameScene() : score(0) {}
    
    void Initialize() override {
        // Create player
        player = std::make_shared<Player>(400, 300);
        AddGameObject(player);
        
        // Create some collectibles
        for (int i = 0; i < 10; i++) {
            float x = 100 + (i % 5) * 150;
            float y = 100 + (i / 5) * 200;
            auto collectible = std::make_shared<Collectible>(x, y);
            AddGameObject(collectible);
            collectibles.push_back(collectible);
        }
        
        std::cout << "Game initialized! Use WASD or Arrow Keys to move. Collect yellow items!" << std::endl;
    }
    
    void Update(float deltaTime) override {
        Scene::Update(deltaTime);
        
        // Check collectible collisions
        for (auto& collectible : collectibles) {
            if (collectible->CheckCollision(player->position, 30.0f)) {
                score += 10;
                std::cout << "Score: " << score << std::endl;
            }
        }
        
        // Quit on ESC
        if (GetEngine() && GetEngine()->GetInputManager()->IsKeyPressed(SDLK_ESCAPE)) {
            GetEngine()->Quit();
        }
    }
    
private:
    std::shared_ptr<Player> player;
    std::vector<std::shared_ptr<Collectible>> collectibles;
    int score;
};

class GameEngine : public Engine {
public:
    bool Initialize() {
        if (!Engine::Initialize("2D Game Engine - Player Example", 800, 600)) {
            return false;
        }
        
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
        GetRenderer()->Clear(Color(30, 30, 60, 255)); // Dark blue background
        
        if (m_scene) {
            m_scene->Render(GetRenderer());
        }
        
        GetRenderer()->Present();
    }
    
private:
    std::unique_ptr<GameScene> m_scene;
};

int main(int argc, char* argv[]) {
    GameEngine game;
    
    if (!game.Initialize()) {
        std::cout << "Failed to initialize game!" << std::endl;
        return -1;
    }
    
    game.Run();
    return 0;
}
