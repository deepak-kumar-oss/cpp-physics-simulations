#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

constexpr int   COLS        = 30;
constexpr int   ROWS        = 22;
constexpr float REST_LEN    = 22.f;
constexpr int   ITERATIONS  = 8;
constexpr float GRAVITY     = 0.55f;
constexpr float DAMPING     = 0.98f;
constexpr float TEAR_DIST   = REST_LEN * 3.f;
constexpr float DRAG_RADIUS = 20.f;

// ─── Point ───
struct Point {
    sf::Vector2f pos, prev;
    bool pinned = false;
    bool active = true;

    Point(float x, float y, bool pin=false)
        : pos(x,y), prev(x,y), pinned(pin) {}

    void update(float wind) {
        if (pinned || !active) return;

        sf::Vector2f vel = (pos - prev) * DAMPING;
        vel.x += wind * 0.003f;
        vel.y += GRAVITY * 0.1f;

        prev = pos;
        pos += vel;
    }

    void constrain(float W, float H) {
        if (pinned || !active) return;

        if (pos.y > H) { pos.y = H; prev.y = H; }
        if (pos.x < 0) pos.x = 0;
        if (pos.x > W) pos.x = W;
    }
};

// ─── Constraint ───
struct Constraint {
    Point* a;
    Point* b;
    float rest;
    bool active = true;

    Constraint(Point* a, Point* b, float r)
        : a(a), b(b), rest(r) {}

    void solve() {
        if (!active || !a->active || !b->active) return;

        sf::Vector2f delta = b->pos - a->pos;
        float dist = std::sqrt(delta.x*delta.x + delta.y*delta.y);
        if (dist < 0.001f) return;

    
        if (dist > TEAR_DIST) {
            active = false;
            return;
        }

        float diff = (dist - rest) / dist * 0.5f;
        sf::Vector2f offset = delta * diff;

        if (!a->pinned) a->pos += offset;
        if (!b->pinned) b->pos -= offset;
    }
};

// ─── Cloth ───
class Cloth {
public:
    std::vector<Point> pts;
    std::vector<Constraint> cons;
    int cols, rows;
    float W, H;

    Cloth(int c, int r, float w, float h)
        : cols(c), rows(r), W(w), H(h)
    {
        build();
    }

    void build() {
        pts.clear(); cons.clear();

        float startX = (W - REST_LEN*(cols-1))/2.f;
        float startY = H * 0.06f;

        for(int r=0;r<rows;r++){
            for(int c=0;c<cols;c++){
                bool pin = (r==0 && c%3==0);
                pts.emplace_back(startX + c*REST_LEN,
                                 startY + r*REST_LEN,
                                 pin);
            }
        }

        for(int r=0;r<rows;r++){
            for(int c=0;c<cols;c++){
                int i = r*cols + c;

                if(c<cols-1)
                    cons.emplace_back(&pts[i], &pts[i+1], REST_LEN);

                if(r<rows-1)
                    cons.emplace_back(&pts[i], &pts[i+cols], REST_LEN);
            }
        }
    }

    void update(float wind) {
        for(auto &p: pts) p.update(wind);

        for(int i=0;i<ITERATIONS;i++)
            for(auto &c: cons) c.solve();

        for(auto &p: pts) p.constrain(W,H);
    }

    Point* nearest(sf::Vector2f mouse) {
        Point* best=nullptr;
        float bestD = DRAG_RADIUS*DRAG_RADIUS;

        for(auto &p: pts){
            if(p.pinned || !p.active) continue;

            sf::Vector2f d = p.pos - mouse;
            float dist2 = d.x*d.x + d.y*d.y;

            if(dist2 < bestD){
                bestD = dist2;
                best = &p;
            }
        }
        return best;
    }

    void tearAt(sf::Vector2f pos) {
        float r2 = 15.f*15.f;

        for(auto &c: cons){
            sf::Vector2f mid = (c.a->pos + c.b->pos)*0.5f;
            sf::Vector2f d = mid - pos;

            if(d.x*d.x + d.y*d.y < r2)
                c.active = false;
        }
    }

    void draw(sf::RenderWindow& win) {

        sf::VertexArray lines(sf::PrimitiveType::Lines);

        for(auto &c: cons){
            if(!c.active || !c.a->active || !c.b->active) continue;

            sf::Vector2f d = c.b->pos - c.a->pos;
            float stretch = std::sqrt(d.x*d.x + d.y*d.y)/c.rest;
            stretch = std::clamp(stretch,1.f,2.f) - 1.f;

            uint8_t r = (uint8_t)(80 + stretch*175);
            uint8_t g = (uint8_t)(120 - stretch*90);
            uint8_t b = (uint8_t)(220 - stretch*200);

            sf::Color col(r,g,b,200);

            sf::Vertex v1;
            v1.position = c.a->pos;
            v1.color = col;

            sf::Vertex v2;
            v2.position = c.b->pos;
            v2.color = col;

            lines.append(v1);
            lines.append(v2);
        }

        win.draw(lines);

        // pins
        sf::CircleShape dot(4.f);
        dot.setOrigin({4.f,4.f});
        dot.setFillColor(sf::Color(230,210,150));

        for(auto &p: pts){
            if(p.pinned){
                dot.setPosition(p.pos);
                win.draw(dot);
            }
        }
    }
};

// ─── MAIN ───
int main(){
    sf::RenderWindow window(sf::VideoMode({900,650}), "Cloth Simulation 🔥");
    window.setFramerateLimit(60);

    Cloth cloth(COLS, ROWS, 900, 650);

    Point* dragPt = nullptr;
    bool tearMode = false;
    float wind = 0;

    while(window.isOpen()){

        while(auto event = window.pollEvent()){
            if(event->is<sf::Event::Closed>())
                window.close();
        }

        // mouse
        sf::Vector2i m = sf::Mouse::getPosition(window);
        sf::Vector2f mp((float)m.x,(float)m.y);

        if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)){
            if(!dragPt) dragPt = cloth.nearest(mp);
        } else dragPt = nullptr;

        if(dragPt){
            dragPt->pos = mp;
            dragPt->prev = mp;
            if(tearMode) cloth.tearAt(mp);
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T))
            tearMode = true;
        else
            tearMode = false;

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            wind -= 1.f;

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            wind += 1.f;

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            wind = 0;

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
            cloth.build();

        cloth.update(wind);

        window.clear(sf::Color(10,10,16));
        cloth.draw(window);
        window.display();
    }
}