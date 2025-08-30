#include "Physics.h"
#include <cmath>

void Physics::UpdateBody(Body& body, float deltaTime) {
    if (body.isStatic) return;
    
    // Update velocity with acceleration
    body.velocity.x += body.acceleration.x * deltaTime;
    body.velocity.y += body.acceleration.y * deltaTime;
    
    // Update position with velocity
    body.position.x += body.velocity.x * deltaTime;
    body.position.y += body.velocity.y * deltaTime;
    
    // Reset acceleration for next frame
    body.acceleration.x = 0;
    body.acceleration.y = 0;
}

void Physics::ApplyGravity(Body& body, const Vector2& gravity) {
    if (!body.isStatic) {
        body.acceleration.x += gravity.x;
        body.acceleration.y += gravity.y;
    }
}

bool Physics::CheckCollision(const AABB& a, const AABB& b) {
    return a.Intersects(b);
}

void Physics::ResolveCollision(Body& a, Body& b, const AABB& aabb1, const AABB& aabb2) {
    // Calculate overlap
    float overlapX = std::min(aabb1.max.x, aabb2.max.x) - std::max(aabb1.min.x, aabb2.min.x);
    float overlapY = std::min(aabb1.max.y, aabb2.max.y) - std::max(aabb1.min.y, aabb2.min.y);
    
    // Determine collision direction (smallest overlap)
    if (overlapX < overlapY) {
        // Horizontal collision
        float separationX = overlapX / 2.0f;
        if (aabb1.min.x < aabb2.min.x) {
            separationX = -separationX;
        }
        
        if (!a.isStatic) a.position.x -= separationX;
        if (!b.isStatic) b.position.x += separationX;
        
        // Exchange velocities with restitution
        if (!a.isStatic && !b.isStatic) {
            float relativeVelocity = a.velocity.x - b.velocity.x;
            float impulse = relativeVelocity / (1.0f/a.mass + 1.0f/b.mass);
            float restitution = (a.restitution + b.restitution) / 2.0f;
            
            a.velocity.x -= impulse * (1 + restitution) / a.mass;
            b.velocity.x += impulse * (1 + restitution) / b.mass;
        }
    } else {
        // Vertical collision
        float separationY = overlapY / 2.0f;
        if (aabb1.min.y < aabb2.min.y) {
            separationY = -separationY;
        }
        
        if (!a.isStatic) a.position.y -= separationY;
        if (!b.isStatic) b.position.y += separationY;
        
        // Exchange velocities with restitution
        if (!a.isStatic && !b.isStatic) {
            float relativeVelocity = a.velocity.y - b.velocity.y;
            float impulse = relativeVelocity / (1.0f/a.mass + 1.0f/b.mass);
            float restitution = (a.restitution + b.restitution) / 2.0f;
            
            a.velocity.y -= impulse * (1 + restitution) / a.mass;
            b.velocity.y += impulse * (1 + restitution) / b.mass;
        }
    }
}
