#pragma once
#include <SFML/Graphics.hpp>

enum class ParticleType { Spark, Ember, Smoke };

struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Vector2f accel   = {0.f, 60.f};  // gravity
    float        lifetime    = 0.f;
    float        maxLifetime = 1.f;
    float        startSize   = 4.f;
    ParticleType type        = ParticleType::Spark;
    bool         alive       = false;

    float t() const { return lifetime / maxLifetime; }  // 0=born → 1=dead
};