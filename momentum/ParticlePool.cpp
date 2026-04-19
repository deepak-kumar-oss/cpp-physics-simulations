#include "ParticlePool.h"
#include <cmath>
#include <cstdlib>

static float randf(float lo, float hi) {
    return lo + (hi - lo) * (std::rand() / float(RAND_MAX));
}
static float randAngle() { return randf(0.f, 2.f * 3.14159f); }

Particle* ParticlePool::next() {
    for (auto& p : pool)
        if (!p.alive) return &p;
    return nullptr;  // pool full — drop particle
}

void ParticlePool::emit(sf::Vector2f pos, sf::Vector2f vel,
                        float lifetime, float size, ParticleType type) {
    Particle* p = next();
    if (!p) return;
    p->pos         = pos;
    p->vel         = vel;
    p->accel       = (type == ParticleType::Smoke)
                       ? sf::Vector2f{randf(-8,8), -20.f}   // smoke drifts up
                       : sf::Vector2f{randf(-5,5),  80.f};  // sparks/embers fall
    p->lifetime    = 0.f;
    p->maxLifetime = lifetime;
    p->startSize   = size;
    p->type        = type;
    p->alive       = true;
}

// ── Preset emitters ──────────────────────────────────────────────────────────

void ParticlePool::spawnExplosion(sf::Vector2f pos, int count) {
    for (int i = 0; i < count; i++) {
        float angle = randAngle();
        float speed = randf(40.f, 260.f);
        sf::Vector2f vel{ std::cos(angle) * speed, std::sin(angle) * speed };

        // Mix of fast sparks and slower embers
        bool isSpark = (i % 3 != 0);
        emit(pos, vel,
             randf(0.4f, isSpark ? 0.9f : 1.4f),
             randf(isSpark ? 2.f : 4.f, isSpark ? 5.f : 9.f),
             isSpark ? ParticleType::Spark : ParticleType::Ember);
    }
    spawnSmoke(pos, 10);  // smoke cap on top
}

void ParticlePool::spawnSparks(sf::Vector2f pos,
                               sf::Vector2f normal, int count) {
    for (int i = 0; i < count; i++) {
        float spread = randf(-1.1f, 1.1f);
        float speed  = randf(80.f, 220.f);
        // Reflect around surface normal with random spread
        sf::Vector2f dir = normal + sf::Vector2f{-normal.y * spread, normal.x * spread};
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (len > 0.001f) dir /= len;
        emit(pos, dir * speed,
             randf(0.2f, 0.6f),
             randf(1.5f, 3.5f),
             ParticleType::Spark);
    }
}

void ParticlePool::spawnSmoke(sf::Vector2f pos, int count) {
    for (int i = 0; i < count; i++) {
        sf::Vector2f vel{ randf(-20.f, 20.f), randf(-60.f, -20.f) };
        emit(pos, vel,
             randf(0.8f, 2.0f),
             randf(8.f, 20.f),
             ParticleType::Smoke);
    }
}

// ── Update ───────────────────────────────────────────────────────────────────

void ParticlePool::update(float dt) {
    for (auto& p : pool) {
        if (!p.alive) continue;
        p.lifetime += dt;
        if (p.lifetime >= p.maxLifetime) { p.alive = false; continue; }
        p.vel += p.accel * dt;
        // Light drag so things don't accelerate forever
        p.vel *= (1.f - 1.5f * dt);
        p.pos += p.vel * dt;
    }
}

// ── Render (VertexArray quad per particle) ───────────────────────────────────

sf::Color ParticlePool::colorFor(const Particle& p) {
    float t = p.t();  // 0=just born, 1=about to die
    uint8_t a = (uint8_t)(255.f * (1.f - t) * (1.f - t));  // quadratic fade

    switch (p.type) {
    case ParticleType::Spark:
        // white-yellow → orange → red
        return {
            255,
            (uint8_t)(255.f * (1.f - t * 0.85f)),
            (uint8_t)(200.f * (1.f - t)),
            a
        };
    case ParticleType::Ember:
        // deep orange → dark red, slower fade
        return {
            (uint8_t)(255.f * (1.f - t * 0.3f)),
            (uint8_t)(120.f * (1.f - t)),
            0,
            (uint8_t)(a * 0.85f)
        };
    case ParticleType::Smoke:
        // gray, grows, fades
        uint8_t g = (uint8_t)(60 + 80 * t);
        return { g, g, g, (uint8_t)(120.f * (1.f - t)) };
    }
    return sf::Color::White;
}

void ParticlePool::draw(sf::RenderTarget& target) {
    verts.clear();

    for (const auto& p : pool) {
        if (!p.alive) continue;

        float t    = p.t();
        // Sparks shrink; smoke grows
        float size = (p.type == ParticleType::Smoke)
                       ? p.startSize * (0.5f + 1.5f * t)
                       : p.startSize * (1.f - t * 0.7f);
        if (size < 0.3f) continue;

        sf::Color c = colorFor(p);
        sf::Vector2f o{size, size};

        // Two triangles forming a quad
        sf::Vertex tl{{p.pos.x - size, p.pos.y - size}, c};
        sf::Vertex tr{{p.pos.x + size, p.pos.y - size}, c};
        sf::Vertex br{{p.pos.x + size, p.pos.y + size}, c};
        sf::Vertex bl{{p.pos.x - size, p.pos.y + size}, c};

        verts.append(tl); verts.append(tr); verts.append(br);
        verts.append(tl); verts.append(br); verts.append(bl);
    }

    // Additive blend makes overlapping sparks glow brighter
    sf::RenderStates states;
    states.blendMode = sf::BlendAdd;
    target.draw(verts, states);
}