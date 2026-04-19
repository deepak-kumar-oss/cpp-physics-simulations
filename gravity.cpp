#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <cmath>
#include<string>

std::vector<sf::Color> colors = {sf::Color::Green , sf::Color::Red , sf::Color::Blue , sf::Color::Yellow , sf::Color::White};

struct Ball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

int main()
{

    sf::RenderWindow window(sf::VideoMode({800, 800}), "Gravity");
    // float width = window.getSize().x;
    // float height = window.getSize().y;
    // sf::CircleShape ball;
    // ball.setRadius(50);
    // ball.setPosition({300 , 300});
    // ball.setFillColor(sf::Color::Blue);

    std::vector<Ball> balls;

    for (int i = 0; i < 5; i++)
    {
        Ball b;
        b.shape = sf::CircleShape(30);
        // b.shape.setFillColor(colors[i%colors.size()]);
        b.shape.setFillColor(sf::Color(rand()%255, rand()%255, rand()%255));
        b.shape.setPosition({100.f + i * 100.f, 100.f + i*100.f});

        b.velocity = {0.2f, 0.0f}; // initial velocity
        balls.push_back(b);
    }

    // float velocityY = 0.0f;
    float gravity = 0.001f;
    float bounce = 0.95f;

    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // velocityY += gravity;
        // ball.move({0 , velocityY});
        // float y = ball.getPosition().y;
        // float radius = ball.getRadius();

        // // floor collision
        // if(y + radius*2 >= 800){
        //     ball.setPosition({ball.getPosition().x , 800-radius*2});
        //     velocityY = -velocityY*bounce;
        // }
        window.clear();
        float width = window.getSize().x;
        float height = window.getSize().y;

        for (auto &b : balls)
        {

            b.velocity.y += gravity;
            b.shape.move(b.velocity);

            sf::Vector2f pos = b.shape.getPosition();
            float r = b.shape.getRadius();

            if (pos.x <= 0)
            {
                b.shape.setPosition({0, pos.y});
                b.velocity.x = -b.velocity.x;
            }

            if (pos.x + r * 2 >= width)
            {
                b.shape.setPosition({width - r * 2, pos.y});
                b.velocity.x = -b.velocity.x;
            }

           
            if (pos.y <= 0)
            {
                b.shape.setPosition({pos.x, 0});
                b.velocity.y = -b.velocity.y;
            }

            if (pos.y + r * 2 >= height)
            {
                b.shape.setPosition({pos.x, height - r * 2});
                b.velocity.y = -b.velocity.y * bounce;

                b.velocity.x *= 0.8f;

                if (abs(b.velocity.y) < 0.01f)
                    b.velocity.y = 0;
            }

            for (int i = 0; i < balls.size() ; i++)
            {
                for (int j = i + 1; j < balls.size(); j++)
                {
                    sf::Vector2f pos1 = balls[i].shape.getPosition();
                    sf::Vector2f pos2 = balls[j].shape.getPosition();

                    float r1 = balls[i].shape.getRadius();
                    float r2 = balls[j].shape.getRadius();
                    
                    sf::Vector2f c1 = pos1 + sf::Vector2f(r1 , r1);
                    sf::Vector2f c2 = pos2 + sf::Vector2f(r2 , r2);

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

                        balls[i].velocity = v2;
                        balls[j].velocity = v1;
                    }

                }
            }

            window.draw(b.shape);
        }

        window.display();
    }
}