#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float mass;
};

int main(){
    sf::RenderWindow window(sf::VideoMode({800 , 800}) , "Mass Physics");

    std::vector<Ball> balls;

    
    for(int i = 0; i < 6; i++){
        Ball b;
        float radius = 20 + rand()%20;   
        b.shape = sf::CircleShape(radius);

        b.shape.setFillColor(sf::Color(rand()%255, rand()%255, rand()%255));
        b.shape.setPosition({100.f + i * 100.f, 100.f});

        b.velocity = { (rand()%5 - 2) * 0.3f, 0.0f };

        b.mass = radius; 

        balls.push_back(b);
    }

    float gravity = 0.001f;
    float bounce = 0.9f;

    while(window.isOpen()){

        while (auto event = window.pollEvent()){
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        float width = window.getSize().x;
        float height = window.getSize().y;

        window.clear();

      
        for(auto &b : balls){

            // gravity
            b.velocity.y += gravity;

            // move
            b.shape.move(b.velocity);

            sf::Vector2f pos = b.shape.getPosition();
            float r = b.shape.getRadius();

            // LEFT
            if (pos.x <= 0) {
                b.shape.setPosition({0, pos.y});
                b.velocity.x = -b.velocity.x;
            }

            // RIGHT
            if (pos.x + r*2 >= width) {
                b.shape.setPosition({width - r*2, pos.y});
                b.velocity.x = -b.velocity.x;
            }

            // TOP
            if (pos.y <= 0) {
                b.shape.setPosition({pos.x, 0});
                b.velocity.y = -b.velocity.y;
            }

            // FLOOR
            if (pos.y + r*2 >= height) {
                b.shape.setPosition({pos.x, height - r*2});
                b.velocity.y = -b.velocity.y * bounce;

                // friction
                b.velocity.x *= 0.98f;

                // stop jitter
                if (std::abs(b.velocity.y) < 0.01f)
                    b.velocity.y = 0;
            }
        }

     
        for (int i = 0; i < balls.size(); i++) {
            for (int j = i + 1; j < balls.size(); j++) {

                sf::Vector2f pos1 = balls[i].shape.getPosition();
                sf::Vector2f pos2 = balls[j].shape.getPosition();

                float r1 = balls[i].shape.getRadius();
                float r2 = balls[j].shape.getRadius();

                sf::Vector2f c1 = pos1 + sf::Vector2f(r1, r1);
                sf::Vector2f c2 = pos2 + sf::Vector2f(r2, r2);

                sf::Vector2f diff = c2 - c1;

                float distSq = diff.x * diff.x + diff.y * diff.y;
                float minDist = r1 + r2;

                if (distSq < minDist * minDist) {

                    float dist = std::sqrt(distSq);
                    if (dist == 0) dist = 0.1f;

                    sf::Vector2f normal = diff / dist;

                    float overlap = 0.5f * (minDist - dist);
                    balls[i].shape.move(-normal * overlap);
                    balls[j].shape.move(normal * overlap);

                 

                    sf::Vector2f v1 = balls[i].velocity;
                    sf::Vector2f v2 = balls[j].velocity;

                    float m1 = balls[i].mass;
                    float m2 = balls[j].mass;

                    sf::Vector2f relVel = v1 - v2;

                    float velAlongNormal = relVel.x * normal.x + relVel.y * normal.y;

                    
                    if (velAlongNormal > 0)
                        continue;

                    float e = 0.9f; 

                    float jImpulse = -(1 + e) * velAlongNormal;
                    jImpulse /= (1/m1 + 1/m2);

                    sf::Vector2f impulse = jImpulse * normal;

                    balls[i].velocity += impulse / m1;
                    balls[j].velocity -= impulse / m2;
                }
            }
        }

        
        for(auto &b : balls){
            window.draw(b.shape);
        }

        window.display();
    }
}