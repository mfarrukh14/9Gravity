#pragma once

#include "Renderer.h"

class Physics {
public:
    struct Body {
        Vector2 position;
        Vector2 velocity;
        Vector2 acceleration;
        float mass;
        float restitution; // bounciness
        bool isStatic;
        
        Body() : mass(1.0f), restitution(0.5f), isStatic(false) {}
    };
    
    struct AABB {
        Vector2 min, max;
        
        AABB() {}
        AABB(const Vector2& pos, float width, float height) {
            min = Vector2(pos.x - width/2, pos.y - height/2);
            max = Vector2(pos.x + width/2, pos.y + height/2);
        }
        
        bool Intersects(const AABB& other) const {
            return !(max.x < other.min.x || min.x > other.max.x ||
                    max.y < other.min.y || min.y > other.max.y);
        }
    };
    
    static void UpdateBody(Body& body, float deltaTime);
    static void ApplyGravity(Body& body, const Vector2& gravity);
    static bool CheckCollision(const AABB& a, const AABB& b);
    static void ResolveCollision(Body& a, Body& b, const AABB& aabb1, const AABB& aabb2);
};
