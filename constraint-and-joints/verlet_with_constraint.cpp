// core formula x(new) = x  + (x - x(prev)) + a*dt*dt
// x - x(pre) -> velocity 
// + a*dt*dt -> acceleration 
// this gives new postion 


// now lets add constraints to connect particles together like a rope or cloth
// contraint = all particles should be at a certain distance from each other and if they are not we will move them to satisfy that constraint

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

struct Particle{
    sf::Vector2f pos;
    sf::Vector2f prevPos;
    int mass;
};

struct Constraint{
    int p1 , p2;
    float restLength;
};

    

std::vector<Particle> particles;
std::vector<Constraint> constraints;

void update(float deltaTime){
    for(int i = 0 ;i < particles.size() ;i++){
        Particle &p  = particles[i];
        sf::Vector2f force = {0.0f , 200.0f};
        sf::Vector2f acceleration = {force.x/p.mass , force.y/p.mass};

        sf::Vector2f previousPos = {p.pos.x , p.pos.y};

        p.pos.x = 2*p.pos.x - p.prevPos.x + acceleration.x*(deltaTime*deltaTime);
        p.pos.y = 2*p.pos.y - p.prevPos.y + acceleration.y*(deltaTime*deltaTime);

        p.prevPos.x = previousPos.x;
        p.prevPos.y = previousPos.y;



    }
}


void solveConstraints(){
    for(int i = 0; i < constraints.size(); i++){
        Constraint &c = constraints[i];

        Particle &p1 = particles[c.p1];
        Particle &p2 = particles[c.p2];

        sf::Vector2f delta = p2.pos - p1.pos;

        float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if(distance == 0) continue;

        float diff = (distance - c.restLength) / distance;

        p1.pos += delta * 0.5f * diff;
        p2.pos -= delta * 0.5f * diff;

        // keep inside window
    }
}



int main(){
    sf::RenderWindow window(sf::VideoMode({800,600}), "Verlet Integration Demo");
    window.setFramerateLimit(60);

    sf::Clock clock;

    // create particle 

    Particle p1;
    p1.pos = {400.f , 100.f};
    p1.prevPos = p1.pos;
    p1.mass = 1;
    particles.push_back(p1);

    Particle p2;
    p2.pos = {400.f , 200.f};
    p2.prevPos = p2.pos;
    p2.mass = 1;
    particles.push_back(p2);

    Particle p3;
    p3.pos = {300.f , 100.f};
    p3.prevPos = p3.pos;
    p3.mass = 1;
    particles.push_back(p3);

    Particle p4;
    p4.pos = {300.f , 200.f};
    p4.prevPos = p4.pos;
    p4.mass = 1;
    particles.push_back(p4);


    // constraints (like springs)

    constraints.push_back({0, 1, 100.f});
    constraints.push_back({2, 3, 100.f}); 
    constraints.push_back({0, 2, 100.f}); 
    constraints.push_back({1, 3, 100.f}); 

    // diagonals (important for stability)
    constraints.push_back({0, 3, 141.f});
    constraints.push_back({1, 2, 141.f});


    sf::CircleShape circle(10.f);
    circle.setFillColor(sf::Color::Yellow);
    circle.setOrigin({10.f , 10.f});

    while(window.isOpen()){
        while (auto event = window.pollEvent()){
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        float dt = clock.restart().asSeconds();

        update(dt);

        float ground = 550.f;

        for(int i = 0 ;i < particles.size() ;i++){
            Particle &p  = particles[i];
            if(p.pos.y > ground){
                p.pos.y = ground;
                float damping = 1.99f;
                float velocityY = (p.pos.y - p.prevPos.y) * damping;
                p.prevPos.y = p.pos.y - velocityY;
            }
        }

        for(int i = 0; i < 5; i++){
            solveConstraints();
        }

        window.clear();

        for(int i = 0; i < particles.size(); i++){
            circle.setPosition(particles[i].pos);
            window.draw(circle);
        }

        window.display();
    }
}