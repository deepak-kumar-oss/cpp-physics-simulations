#pragma once
#include "Particle.h"
#include <array>

class ParticlePool {
public:
    static constexpr int MAX = 2000;

    void emit(sf::Vector2f pos, sf::Vector2f vel,
              float lifetime, float size, ParticleType type);

    void spawnExplosion(sf::Vector2f pos, int count = 80);
    void spawnSparks   (sf::Vector2f pos, sf::Vector2f surfaceNormal, int count = 30);
    void spawnSmoke    (sf::Vector2f pos, int count = 12);

    void update(float dt);
    void draw  (sf::RenderTarget& target);

private:
    std::array<Particle, MAX> pool;
    sf::VertexArray           verts{sf::PrimitiveType::Triangles};

    Particle* next();   // grab first dead slot
    sf::Color colorFor(const Particle& p);
};