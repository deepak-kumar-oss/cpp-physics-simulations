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


bool colliding(const Ball &a, const Ball &b) {
    sf::Vector2f d = a.shape.getPosition() - b.shape.getPosition();
    float distSq = d.x*d.x + d.y*d.y;

    float r = a.shape.getRadius() + b.shape.getRadius();
    return distSq <= r*r;
}


void resolveCollision(Ball &a, Ball &b) {

    sf::Vector2f posA = a.shape.getPosition();
    sf::Vector2f posB = b.shape.getPosition();

    sf::Vector2f delta = posA - posB;
    float d = std::sqrt(delta.x*delta.x + delta.y*delta.y);

    if (d == 0) d = 0.1f;

    float rA = a.shape.getRadius();
    float rB = b.shape.getRadius();

    sf::Vector2f mtd = delta * ((rA + rB - d) / d);

    float im1 = 1.0f / a.mass;
    float im2 = 1.0f / b.mass;

    // push apart
    a.shape.move(mtd * (im1 / (im1 + im2)));
    b.shape.move(-mtd * (im2 / (im1 + im2)));

    sf::Vector2f v = a.velocity - b.velocity;

    sf::Vector2f normal = mtd;
    float len = std::sqrt(normal.x*normal.x + normal.y*normal.y);
    normal /= len;

    float vn = v.x * normal.x + v.y * normal.y;

    if (vn > 0.0f) return;

    float restitution = 0.9f;

    float impulseMag = (-(1.0f + restitution) * vn) / (im1 + im2);

    sf::Vector2f impulse = normal * impulseMag;

    a.velocity += impulse * im1;
    b.velocity -= impulse * im2;
}

int main(){
    sf::RenderWindow window(sf::VideoMode({800 , 800}) , "Physics Sandbox 🔥");
    window.setFramerateLimit(60);

    std::vector<Ball> balls;

    float gravity = 0.3f;
    float bounce = 0.8f;

    int selectedBall = -1;

    sf::Font font;
    if(!font.openFromFile("LoveDays-2v7Oe.ttf")){
       
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

                // 🗑 DELETE
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

            if(event->is<sf::Event::KeyPressed>()){
                if(event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::R)
                    balls.clear();
            }
        }

     
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

  
        for(auto &b : balls){

            b.velocity.y += gravity;
            b.shape.move(b.velocity);

            sf::Vector2f pos = b.shape.getPosition();
            float r = b.shape.getRadius();

            if(pos.x - r < 0){ b.shape.setPosition({r,pos.y}); b.velocity.x *= -1; }
            if(pos.x + r > W){ b.shape.setPosition({W-r,pos.y}); b.velocity.x *= -1; }
            if(pos.y - r < 0){ b.shape.setPosition({pos.x,r}); b.velocity.y *= -1; }

            if(pos.y + r > H){
                b.shape.setPosition({pos.x,H-r});
                b.velocity.y *= -bounce;
                b.velocity.x *= 0.98f;
            }

            // trail
            b.trail.push_back(pos);
            if(b.trail.size() > 20)
                b.trail.erase(b.trail.begin());
        }

 
        for(int i=0;i<balls.size();i++){
            for(int j=i+1;j<balls.size();j++){
                if(colliding(balls[i], balls[j])){
                    resolveCollision(balls[i], balls[j]);
                }
            }
        }

    
        for(auto &b : balls){

            for(auto &t : b.trail){
                sf::CircleShape dot(2);
                dot.setPosition(t);
                dot.setFillColor(sf::Color(255,255,255,100));
                window.draw(dot);
            }

            window.draw(b.shape);
        }

        // UI
        std::stringstream ss;
        ss << "Balls: " << balls.size() << " | R = reset";
        ui.setString(ss.str());
        window.draw(ui);

        window.display();
    }
}