#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include "ParticlePool.h"

struct ball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float mass;
    std::vector<sf::Vector2f> trail;
};

bool colliding(const ball &a, const ball &b)
{
    sf::Vector2f d = a.shape.getPosition() - b.shape.getPosition();
    float dist = d.x * d.x + d.y * d.y;
    float r = a.shape.getRadius() + b.shape.getRadius();

    return dist <= r * r;
}

void resolveCollision(ball &a, ball &b)
{
    //
    sf::Vector2f posA = a.shape.getPosition();
    sf::Vector2f posB = b.shape.getPosition();

    sf::Vector2f delta = posA - posB;

    float d = std::sqrt(delta.x * delta.x + delta.y * delta.y);

    if (d == 0)
        d = 0.1f;

    float rA = a.shape.getRadius();
    float rB = b.shape.getRadius();

    // mtd smallest vector needed to seprate them
    sf::Vector2f mtd = delta * ((rA + rB - d) / d); // 0 if not colliding

    float m1 = 1.0f / a.mass;
    float m2 = 1.0f / b.mass; // heavy object = less m2 = less moment

    // push apart

    a.shape.move(mtd * (m1 / (m1 + m2)));
    b.shape.move(-mtd * (m2 / (m1 + m2)));

    sf::Vector2f v = a.velocity - b.velocity;

    sf::Vector2f normal = mtd;

    float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);

    if (len == 0)
        return;

    normal /= len;

    float vn = v.x * normal.x + v.y * normal.y;

    if (vn > 0.0f)
        return;

    float restitution = 0.9f;

    float impulseMag = (-(1.0f + restitution) * vn) / (m1 + m2);

    sf::Vector2f impulse = normal * impulseMag;

    a.velocity += impulse * m1;
    b.velocity -= impulse * m2;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 800}), "Window");

    ParticlePool particles;

    int width = window.getSize().x;
    int height = window.getSize().y;

    std::vector<ball> balls;

    float gravity = 800.0f;
    float bounce = 0.8f;

    // delta time

    sf::Clock clock;

    while (window.isOpen())
    {

        float dt = clock.restart().asSeconds();

        if (dt > 0.02f)
            dt = 0.02f;

        // SFML 3 event loop
        while (auto event = window.pollEvent())
        {

            // Close event
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            // Mouse click
            if (event->is<sf::Event::MouseButtonPressed>())
            {
                auto e = event->getIf<sf::Event::MouseButtonPressed>();

                if (e->button == sf::Mouse::Button::Left)
                {
                    std::cout << "Left Click\n";
                    // create a ball
                    sf::Vector2i mousePosition = sf::Mouse::getPosition();

                    ball b;

                    float radius = 10 + rand() % 40;

                    b.shape.setRadius(radius);
                    b.shape.setFillColor(sf::Color(rand() % 255, rand() % 255, rand() % 255));

                    b.shape.setOrigin({(float)radius, (float)radius});
                    auto mousePos = sf::Mouse::getPosition(window);
                    b.shape.setPosition({(float)mousePos.x, (float)mousePos.y});

                    b.mass = radius;
                    b.velocity = {(rand() % 5 - 2) * 2.f, 0.f};

                    balls.push_back(b);
                }

                if (e->button == sf::Mouse::Button::Right)
                {
                    std::cout << "Right Click Delete Ball" << std::endl;
                    sf::Vector2i mousePositon = sf::Mouse::getPosition(window);
                    for (int i = 0; i < balls.size(); i++)
                    {
                        sf::Vector2f pos = balls[i].shape.getPosition();
                        float radius = balls[i].shape.getRadius();

                        float dx = mousePositon.x - pos.x;
                        float dy = mousePositon.y - pos.y;

                        float distance = std::sqrt(dx * dx + dy * dy);

                        if (distance <= radius)
                        {
                            balls[i] = balls.back();
                            balls.pop_back();
                            break;
                        }
                    }
                }
            }
            // balls movement

            // Key press
            if (event->is<sf::Event::KeyPressed>())
            {
                auto e = event->getIf<sf::Event::KeyPressed>();

                if (e->code == sf::Keyboard::Key::Escape)
                {
                    window.close();
                }
            }
        }
        window.clear();

        for (auto &b : balls)
        {
            b.velocity.y += gravity * dt;
            b.shape.move(b.velocity * dt);

            // collisions
            sf::Vector2f pos = b.shape.getPosition();
            float radius = b.shape.getRadius();

            if (pos.x - radius < 0)
            {
                b.shape.setPosition({radius, pos.y});
                b.velocity.x *= -1;
                 particles.spawnSparks(pos, {1.f, 0.f}, 20);
            }
            if (pos.x + radius > width)
            {
                b.shape.setPosition({width - radius, pos.y});
                b.velocity.x *= -1;
                 particles.spawnSparks(pos, {1.f, 0.f}, 20);
            }
            if (pos.y - radius < 0)
            {
                b.shape.setPosition({pos.x, radius});
                b.velocity.y *= -1;
                particles.spawnSparks(pos, {0.f, 1.f}, 20);
            }

            if (pos.y + radius > height)
            {
                b.shape.setPosition({pos.x, height - radius});
                b.velocity.y *= -1 * bounce;
                b.velocity.x *= 0.98f;
                particles.spawnSparks(pos, {0.f, -1.f}, 25);
            }

            b.trail.push_back(pos);
            if (b.trail.size() > 200)
                b.trail.erase(b.trail.begin());
        }

        // ball to ball collisons
        // idea conservation of momentum
        for (int i = 0; i < balls.size(); i++)
        {
            for (int j = i + 1; j < balls.size(); j++)
            {
                if (colliding(balls[i], balls[j]))
                {
                    resolveCollision(balls[i], balls[j]);
                    sf::Vector2f posA = balls[i].shape.getPosition();
                    sf::Vector2f posB = balls[j].shape.getPosition();
                    sf::Vector2f collisionPos = (posA + posB) * 0.5f;

                    particles.spawnExplosion(collisionPos, 60);
                }
            }
        }
        particles.update(dt);


        for (auto &b : balls)
        {
            window.draw(b.shape);
        }

        particles.draw(window);
        window.display();
    }

    return 0;
}