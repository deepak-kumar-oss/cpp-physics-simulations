#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

float length(sf::Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(sf::Vector2f v) {
    float len = length(v);
    if (len == 0) return {0.f, 0.f};
    return v / len;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Collision Physics");

    std::vector<Ball> balls;

    float gravity = 800.f;
    float bounce = -0.7f;

    sf::Clock clock;

    bool dragging = false;
    sf::Vector2f dragStart;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Start drag
            if (event->is<sf::Event::MouseButtonPressed>()) {
                auto mouse = event->getIf<sf::Event::MouseButtonPressed>();
                if (mouse && mouse->button == sf::Mouse::Button::Left) {
                    dragging = true;
                    dragStart = {(float)mouse->position.x, (float)mouse->position.y};
                }
            }

            // Release → spawn ball
            if (event->is<sf::Event::MouseButtonReleased>()) {
                auto mouse = event->getIf<sf::Event::MouseButtonReleased>();
                if (mouse && mouse->button == sf::Mouse::Button::Left) {
                    dragging = false;

                    sf::Vector2f dragEnd = {(float)mouse->position.x, (float)mouse->position.y};
                    sf::Vector2f velocity = (dragStart - dragEnd) * 2.f;

                    Ball b;
                    b.shape = sf::CircleShape(15.f);
                    b.shape.setFillColor(sf::Color::Yellow);
                    b.shape.setPosition(dragStart);
                    b.velocity = velocity;

                    balls.push_back(b);
                }
            }
        }

        // Physics update
        for (auto &b : balls) {
            b.velocity.y += gravity * dt;
            b.shape.move(b.velocity * dt);

            auto pos = b.shape.getPosition();
            float r = b.shape.getRadius();

            // Floor
            if (pos.y + r * 2 >= 600) {
                b.shape.setPosition({pos.x, 600 - r * 2});
                b.velocity.y *= bounce;
            }

            // Walls
            if (pos.x <= 0) {
                b.shape.setPosition({0, pos.y});
                b.velocity.x *= bounce;
            }
            if (pos.x + r * 2 >= 800) {
                b.shape.setPosition({800 - r * 2, pos.y});
                b.velocity.x *= bounce;
            }
        }

        // 🔥 BALL COLLISION
        for (int i = 0; i < balls.size(); i++) {
            for (int j = i + 1; j < balls.size(); j++) {

                auto pos1 = balls[i].shape.getPosition();
                auto pos2 = balls[j].shape.getPosition();

                float r1 = balls[i].shape.getRadius();
                float r2 = balls[j].shape.getRadius();

                sf::Vector2f center1 = pos1 + sf::Vector2f(r1, r1);
                sf::Vector2f center2 = pos2 + sf::Vector2f(r2, r2);

                sf::Vector2f diff = center2 - center1;
                float dist = length(diff);

                float minDist = r1 + r2;

                // Collision detected
                if (dist < minDist && dist != 0) {

                    sf::Vector2f normal = normalize(diff);

                    // Push balls apart (fix overlap)
                    float overlap = minDist - dist;
                    balls[i].shape.move(-normal * (overlap / 2.f));
                    balls[j].shape.move(normal * (overlap / 2.f));

                    // Relative velocity
                    sf::Vector2f relVel = balls[j].velocity - balls[i].velocity;

                    float velAlongNormal = relVel.x * normal.x + relVel.y * normal.y;

                    if (velAlongNormal > 0)
                        continue;

                    float restitution = 0.8f; // bounciness

                    float impulse = -(1 + restitution) * velAlongNormal;
                    impulse /= 2; // equal mass

                    sf::Vector2f impulseVec = impulse * normal;

                    balls[i].velocity -= impulseVec;
                    balls[j].velocity += impulseVec;
                }
            }
        }

        window.clear(sf::Color::Black);

        for (auto &b : balls)
            window.draw(b.shape);

        window.display();
    }

    return 0;
}