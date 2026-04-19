#include <SFML/Graphics.hpp> 

int main(){
    sf::RenderWindow window(sf::VideoMode({800 , 800}) , "WInnIIN");

    sf::CircleShape circle(50);
    circle.setFillColor(sf::Color::Magenta);
    circle.setPosition({30 , 30});

    sf::RectangleShape rectangle({150 , 150});
    rectangle.setFillColor({sf::Color::Red});
    rectangle.setPosition({190,190});

    sf::ConvexShape triangle;
    triangle.setPointCount(3);
    triangle.setPoint(0  , {0,0});
    triangle.setPoint(1  , {100 , 0});
    triangle.setPoint(2  , {50 , 100});

    triangle.setFillColor(sf::Color::Yellow);
    triangle.setPosition({500 , 500});


    sf::Font font;
    if(!font.openFromFile("LoveDays-2v7Oe.ttf")){
        return -1;
    }

    sf::Text text(font);
    text.setString("Hello Bbg");

    float speed = 0.2f;

    sf::Vector2f velocity = {0.3f , 0.25f};



    while(window.isOpen()){
      
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // if(text.getPosition().x > 500 || text.getPosition().x < 0) speed = -speed;

        // if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)){
        //     circle.move({0 , -speed});
        // }
        // if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)){
        //     circle.move({0 , speed});
        // }
        // if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)){
        //     circle.move({-speed , 0});
        // }
        // if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)){
        //     circle.move({speed , 0});
        // }

        circle.move(velocity);

    
        // get position of circle 
        sf::Vector2f pos = circle.getPosition();
        float radius = circle.getRadius();

        // if(pos.x < 0){
        //     circle.setPosition({0 , pos.y});
        // }
        // if(pos.y < 0){
        //     circle.setPosition({pos.x , 0});
        // }
        // if(pos.x + circle.getRadius()*2 > 800){
        //     circle.setPosition({800 - circle.getRadius()*2 , pos.y});
        // }
        // if(pos.y + circle.getRadius()*2 > 800){
        //      circle.setPosition({pos.x , 800 - circle.getRadius()*2 });
        // }

        if(pos.x <= 0 || pos.x + radius*2 >= 800){
            velocity.x = -velocity.x;
        }
        if(pos.y <= 0 || pos.y + radius*2 >= 800){
            velocity.y = -velocity.y;
        }

        // text.move({speed , 0});
        window.clear();
         window.draw(circle);
        // window.draw(rectangle);
        // window.draw(triangle);
        // window.draw(text);

        window.display();
       // sf::sleep(sf::milliseconds(100));  // one way to keep window open
    }
}