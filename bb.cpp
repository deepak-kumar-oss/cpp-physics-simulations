#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <sstream>

struct Ball {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float mass;

    std::vector<sf::Vector2f> trail;
};

int main(){
    sf::RenderWindow window(sf::VideoMode({800 , 800}) , "Physics Sandbox 🔥");
    window.setFramerateLimit(60);

    std::vector<Ball> balls;

    float gravity = 0.3f;
    float bounce = 0.8f;

    int selectedBall = -1;

    // 🔤 FONT (make sure font exists or replace path)
    sf::Font font;
    if(!font.openFromFile("LoveDays-2v7Oe.ttf")){
        // Handle error
        return -1;
    }

    sf::Text ui(font);
    ui.setCharacterSize(18);
    ui.setFillColor(sf::Color::White);
    ui.setPosition({10, 10});

    while(window.isOpen()){

        while(auto event = window.pollEvent()){
            if(event->is<sf::Event::Closed>())
                window.close();

            // 🖱️ LEFT CLICK → SPAWN
            if(event->is<sf::Event::MouseButtonPressed>()){
                auto e = event->getIf<sf::Event::MouseButtonPressed>();

                sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

                if(e->button == sf::Mouse::Button::Left){
                    Ball b;

                    float r = 10 + rand()%30;
                    b.shape = sf::CircleShape(r);
                    b.shape.setOrigin({r,r});
                    b.shape.setPosition(mousePos);
                    b.shape.setFillColor(sf::Color(rand()%255, rand()%255, rand()%255));

                    b.velocity = {(rand()%5 - 2)*2.f, 0.f};
                    b.mass = r;

                    balls.push_back(b);
                }

                // 🗑️ MIDDLE CLICK → DELETE
                if(e->button == sf::Mouse::Button::Middle){
                    for(int i=0;i<balls.size();i++){
                        sf::Vector2f pos = balls[i].shape.getPosition();
                        float r = balls[i].shape.getRadius();

                        float dx = pos.x - mousePos.x;
                        float dy = pos.y - mousePos.y;

                        if(dx*dx + dy*dy < r*r){
                            balls.erase(balls.begin()+i);
                            break;
                        }
                    }
                }
            }

            if(event->is<sf::Event::MouseButtonReleased>())
                selectedBall = -1;

            // 🔄 RESET
            if(event->is<sf::Event::KeyPressed>()){
                if(event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::R){
                    balls.clear();
                }
            }
        }

        // 🖱️ RIGHT CLICK DRAG
        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)){
            if(selectedBall == -1){
                sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

                for(int i=0;i<balls.size();i++){
                    sf::Vector2f pos = balls[i].shape.getPosition();
                    float r = balls[i].shape.getRadius();

                    float dx = pos.x - mousePos.x;
                    float dy = pos.y - mousePos.y;

                    if(dx*dx + dy*dy < r*r){
                        selectedBall = i;
                        break;
                    }
                }
            }
        } else selectedBall = -1;

        if(selectedBall != -1){
            sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
            balls[selectedBall].shape.setPosition(mousePos);
            balls[selectedBall].velocity = {0,0};
        }

        window.clear();

        float W = window.getSize().x;
        float H = window.getSize().y;

        // 🔁 UPDATE
        for(auto &b : balls){

            b.velocity.y += gravity;
            b.shape.move(b.velocity);

            sf::Vector2f pos = b.shape.getPosition();
            float r = b.shape.getRadius();

            // walls
            if(pos.x - r < 0){ b.shape.setPosition({r,pos.y}); b.velocity.x *= -1; }
            if(pos.x + r > W){ b.shape.setPosition({W-r,pos.y}); b.velocity.x *= -1; }
            if(pos.y - r < 0){ b.shape.setPosition({pos.x,r}); b.velocity.y *= -1; }

            if(pos.y + r > H){
                b.shape.setPosition({pos.x,H-r});
                b.velocity.y *= -bounce;
                b.velocity.x *= 0.98f;
            }

            // ✨ trail
            b.trail.push_back(pos);
            if(b.trail.size() > 20)
                b.trail.erase(b.trail.begin());
        }

        // 🔥 BALL COLLISION
        for(int i=0;i<balls.size();i++){
            for(int j=i+1;j<balls.size();j++){

                sf::Vector2f p1 = balls[i].shape.getPosition();
                sf::Vector2f p2 = balls[j].shape.getPosition();

                float r1 = balls[i].shape.getRadius();
                float r2 = balls[j].shape.getRadius();

                sf::Vector2f diff = p2 - p1;
                float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y);

                if(dist < r1 + r2 && dist != 0){

                    sf::Vector2f normal = diff / dist;

                    // push apart
                    float overlap = 0.5f * (r1 + r2 - dist);
                    balls[i].shape.move(-normal * overlap);
                    balls[j].shape.move(normal * overlap);

                    // impulse
                    sf::Vector2f relVel = balls[i].velocity - balls[j].velocity;
                    float velAlongNormal = relVel.x*normal.x + relVel.y*normal.y;

                    if(velAlongNormal > 0) continue;

                    float e = 0.9f;
                    float jImp = -(1+e)*velAlongNormal;
                    jImp /= (1/balls[i].mass + 1/balls[j].mass);

                    sf::Vector2f impulse = jImp * normal;

                    balls[i].velocity += impulse / balls[i].mass;
                    balls[j].velocity -= impulse / balls[j].mass;
                }
            }
        }

        // 🎨 DRAW
        for(auto &b : balls){

            // draw trail
            for(int i=0;i<b.trail.size();i++){
                sf::CircleShape dot(2);
                dot.setPosition(b.trail[i]);
                dot.setFillColor(sf::Color(255,255,255,100));
                window.draw(dot);
            }

            window.draw(b.shape);
        }

        // 🧾 UI
        std::stringstream ss;
        ss << "Balls: " << balls.size() << " | R = reset";
        ui.setString(ss.str());
        window.draw(ui);

        window.display();
    }
}