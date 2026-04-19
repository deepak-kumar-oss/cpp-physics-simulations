#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <sstream>

const int WIDTH = 1200;
const int HEIGHT = 900;

// ---------- STRUCTS ----------

struct Enemy {
    sf::Sprite sprite;
    sf::Vector2f velocity;
    float radius;

    Enemy(const sf::Texture& tex) : sprite(tex) {}
};

struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

struct XP {
    sf::CircleShape shape;
};

// ---------- COLLISION (FIXED) ----------

bool colliding(const sf::Vector2f &a, float ra,
               const sf::Vector2f &b, float rb){

    sf::Vector2f d = a - b;
    float distSq = d.x*d.x + d.y*d.y;

    float r = ra + rb + 5.f; // 🔥 buffer added

    return distSq <= r*r;
}

// ---------- MAIN ----------

int main(){
    sf::RenderWindow window(sf::VideoMode({WIDTH,HEIGHT}), "Vampire Clone 🔥");
    window.setFramerateLimit(60);

    // 🧍 PLAYER
    sf::Texture playerTex;
    if(!playerTex.loadFromFile("player.png")) return -1;

    sf::Sprite player(playerTex);

    float playerSize = 60.f;
    auto bounds = player.getLocalBounds();

    player.setScale({
        playerSize / bounds.size.x,
        playerSize / bounds.size.y
    });

    player.setOrigin({bounds.size.x/2.f, bounds.size.y/2.f});
    player.setPosition({WIDTH/2.f, HEIGHT/2.f});

    float playerSpeed = 3.f;

    // 👹 ENEMY
    sf::Texture enemyTex;
    if(!enemyTex.loadFromFile("enemy.png")) return -1;

    std::vector<Enemy> enemies;
    std::vector<Bullet> bullets;
    std::vector<XP> xpOrbs;

    float spawnTimer = 0;
    float spawnDelay = 1.0f;

    float shootTimer = 0;
    float shootDelay = 0.5f;

    sf::Vector2f lastDir = {1.f,0.f};

    int health = 5;
    float score = 0;

    int level = 1;
    int xp = 0;
    int xpToNext = 50;

    bool gameOver = false;

    // 🧾 UI
    sf::Font font;
    if(!font.openFromFile("LoveDays-2v7Oe.ttf")) return -1;

    sf::Text text(font);
    text.setCharacterSize(20);
    text.setFillColor(sf::Color::White);
    text.setPosition({10,10});

    while(window.isOpen()){

        while(auto event = window.pollEvent()){
            if(event->is<sf::Event::Closed>())
                window.close();

            if(gameOver && event->is<sf::Event::KeyPressed>()){
                if(event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::R){
                    enemies.clear();
                    bullets.clear();
                    xpOrbs.clear();

                    player.setPosition({WIDTH/2.f, HEIGHT/2.f});

                    health = 5;
                    score = 0;
                    level = 1;
                    xp = 0;
                    xpToNext = 50;

                    gameOver = false;
                }
            }
        }

        window.clear();

        if(!gameOver){

            // 🎮 MOVEMENT
            sf::Vector2f moveDir = {0,0};

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) moveDir.y -= 1;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) moveDir.y += 1;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) moveDir.x -= 1;
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) moveDir.x += 1;

            if(moveDir.x != 0 || moveDir.y != 0){
                float len = std::sqrt(moveDir.x*moveDir.x + moveDir.y*moveDir.y);
                moveDir /= len;

                player.move(moveDir * playerSpeed);
                lastDir = moveDir;

                float angle = std::atan2(moveDir.y, moveDir.x) * 180 / 3.14159f;
                player.setRotation(sf::degrees(angle + 90.f));
            }

            // 🔫 SHOOT (SPREAD)
            shootTimer += 0.016f;
            if(shootTimer > shootDelay){
                shootTimer = 0;

                int bulletCount = 3;
                float spread = 20.f;

                for(int i=0;i<bulletCount;i++){

                    float offset = (-spread/2.f) + (spread/(bulletCount-1))*i;
                    float base = std::atan2(lastDir.y, lastDir.x);
                    float ang = base + offset * 3.14159f / 180.f;

                    sf::Vector2f dir = {std::cos(ang), std::sin(ang)};

                    Bullet b;
                    b.shape = sf::CircleShape(8); // 🔥 bigger hitbox
                    b.shape.setOrigin({8,8});
                    b.shape.setFillColor(sf::Color::Yellow);

                    b.shape.setPosition(player.getPosition());
                    b.velocity = dir * 6.f; // slower bullets

                    bullets.push_back(b);
                }
            }

            // 👹 SPAWN ENEMY
            spawnTimer += 0.016f;
            if(spawnTimer > spawnDelay){
                spawnTimer = 0;

                Enemy e(enemyTex);

                float size = 80.f;
                auto b = e.sprite.getLocalBounds();

                e.sprite.setScale({size/b.size.x, size/b.size.y});
                e.sprite.setOrigin({b.size.x/2.f, b.size.y/2.f});
                e.radius = size/2.f;

                int side = rand()%4;

                if(side==0) e.sprite.setPosition({0.f, (float)(rand()%HEIGHT)});
                if(side==1) e.sprite.setPosition({(float)WIDTH, (float)(rand()%HEIGHT)});
                if(side==2) e.sprite.setPosition({(float)(rand()%WIDTH), 0.f});
                if(side==3) e.sprite.setPosition({(float)(rand()%WIDTH), (float)HEIGHT});

                e.velocity = {0,0};
                enemies.push_back(e);
            }

            // UPDATE BULLETS
            for(auto &b: bullets)
                b.shape.move(b.velocity);

            // ENEMY AI
            for(int i=0;i<enemies.size();i++){

                sf::Vector2f dir = player.getPosition() - enemies[i].sprite.getPosition();
                float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                if(len!=0) dir/=len;

                enemies[i].velocity += dir * 0.05f;

                float speed = std::sqrt(
                    enemies[i].velocity.x * enemies[i].velocity.x +
                    enemies[i].velocity.y * enemies[i].velocity.y
                );

                float maxSpeed = 2.f + level * 0.2f;

                if(speed > maxSpeed){
                    enemies[i].velocity = (enemies[i].velocity / speed) * maxSpeed;
                }

                enemies[i].sprite.move(enemies[i].velocity);

                if(colliding(player.getPosition(),30,
                             enemies[i].sprite.getPosition(),enemies[i].radius)){
                    health--;
                    enemies.erase(enemies.begin()+i);
                    i--;
                    if(health<=0) gameOver=true;
                }
            }

            // 💥 BULLET COLLISION (FIXED)
            for(int i=0;i<bullets.size();i++){
                for(int j=0;j<enemies.size();j++){

                    if(colliding(bullets[i].shape.getPosition(),8,
                                 enemies[j].sprite.getPosition(),enemies[j].radius)){

                        XP orb;
                        orb.shape = sf::CircleShape(5);
                        orb.shape.setOrigin({5,5});
                        orb.shape.setFillColor(sf::Color::Cyan);
                        orb.shape.setPosition(enemies[j].sprite.getPosition());

                        xpOrbs.push_back(orb);

                        bullets.erase(bullets.begin()+i);
                        enemies.erase(enemies.begin()+j);

                        score += 10;
                        i--;
                        break;
                    }
                }
            }

            // DRAW
            window.draw(player);

            for(auto &b: bullets) window.draw(b.shape);
            for(auto &e: enemies) window.draw(e.sprite);
            for(auto &x: xpOrbs) window.draw(x.shape);

            score += 0.016f;
        }

        std::stringstream ss;

        if(!gameOver){
            ss << "HP: " << health << "  LVL: " << level << "  SCORE: " << (int)score;
        } else {
            ss << "GAME OVER\nScore: " << (int)score << "\nPress R";
        }

        text.setString(ss.str());
        window.draw(text);

        window.display();
    }
}