#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <algorithm>

// ─── Vec2 helpers (SFML already has sf::Vector2f) ───────────────────────────

static float vec2Len(sf::Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

// ─── Forward declarations ────────────────────────────────────────────────────

struct Stick;

// ─── Point ───────────────────────────────────────────────────────────────────

struct Point {
    sf::Vector2f pos;
    sf::Vector2f prevPos;
    sf::Vector2f initPos;
    bool isPinned   = false;
    bool isSelected = false;

    Stick* sticks[2] = { nullptr, nullptr };

    Point() = default;
    Point(float x, float y) : pos(x, y), prevPos(x, y), initPos(x, y) {}

    void addStick(Stick* s, int idx) { sticks[idx] = s; }
    void pin() { isPinned = true; }
    void setPosition(float x, float y) { pos.x = x; pos.y = y; }

    void keepInsideView(int w, int h) {
        if (pos.x < 0)   pos.x = 0;
        if (pos.x > w)   pos.x = (float)w;
        if (pos.y < 0)   pos.y = 0;
        if (pos.y > h)   pos.y = (float)h;
    }

    void update(float dt, float drag, sf::Vector2f gravity, float elasticity,
                sf::Vector2f mousePos, sf::Vector2f mousePrevPos,
                float cursorSize, bool leftDown, bool rightDown,
                int winW, int winH);
};

// ─── Stick ───────────────────────────────────────────────────────────────────

struct Stick {
    Point& p0;
    Point& p1;
    float  length;
    bool   isActive   = true;
    bool   isSelected = false;

    Stick(Point& a, Point& b, float len) : p0(a), p1(b), length(len) {}

    void setSelected(bool v) { isSelected = v; }
    void breakStick()        { isActive = false; }

    void update() {
        if (!isActive) return;
        sf::Vector2f diff = p0.pos - p1.pos;
        float dist = vec2Len(diff);
        if (dist == 0.f) return;
        float factor = (length - dist) / dist;
        sf::Vector2f offset = diff * factor * 0.5f;
        p0.setPosition(p0.pos.x + offset.x, p0.pos.y + offset.y);
        p1.setPosition(p1.pos.x - offset.x, p1.pos.y - offset.y);
    }

    void draw(sf::RenderWindow& window) const {
        if (!isActive) return;
        sf::Color col = isSelected ? sf::Color(0xCC, 0x00, 0x00) : sf::Color::Black;
        sf::Vertex line[2] = {
            sf::Vertex{ p0.pos, col },
            sf::Vertex{ p1.pos, col }
        };
        window.draw(line, 2, sf::PrimitiveType::Lines);
    }
};

// ─── Point::update (needs Stick complete) ────────────────────────────────────

void Point::update(float dt, float drag, sf::Vector2f gravity, float elasticity,
                   sf::Vector2f mousePos, sf::Vector2f mousePrevPos,
                   float cursorSize, bool leftDown, bool rightDown,
                   int winW, int winH)
{
    // selection test (avoid sqrt with squared distance)
    sf::Vector2f d = pos - mousePos;
    float distSq = d.x * d.x + d.y * d.y;
    isSelected = distSq < cursorSize * cursorSize;

    for (Stick* s : sticks)
        if (s) s->setSelected(isSelected);

    if (leftDown && isSelected) {
        sf::Vector2f diff = mousePos - mousePrevPos;
        diff.x = std::clamp(diff.x, -elasticity, elasticity);
        diff.y = std::clamp(diff.y, -elasticity, elasticity);
        prevPos = pos - diff;
    }

    if (rightDown && isSelected) {
        for (Stick* s : sticks)
            if (s) s->breakStick();
    }

    if (isPinned) { pos = initPos; return; }

    sf::Vector2f vel    = pos - prevPos;
    sf::Vector2f newPos = pos + vel * (1.f - drag)
                              + gravity * (1.f - drag) * dt * dt;
    prevPos = pos;
    pos     = newPos;

    keepInsideView(winW, winH);
}

// ─── Cloth ───────────────────────────────────────────────────────────────────

struct Cloth {
    sf::Vector2f gravity   = { 0.f, 981.f };
    float        drag      = 0.01f;
    float        elasticity = 10.f;

    std::vector<Point*> points;
    std::vector<Stick*> sticks;

    Cloth(int width, int height, int spacing, int startX, int startY) {
        for (int y = 0; y <= height; y++) {
            for (int x = 0; x <= width; x++) {
                Point* p = new Point(startX + x * spacing,
                                     startY + y * spacing);
                if (x != 0) {
                    Point* left = points[points.size() - 1];
                    Stick* s = new Stick(*p, *left, (float)spacing);
                    left->addStick(s, 0);
                    p->addStick(s, 0);
                    sticks.push_back(s);
                }
                if (y != 0) {
                    Point* up = points[x + (y - 1) * (width + 1)];
                    Stick* s = new Stick(*p, *up, (float)spacing);
                    up->addStick(s, 1);
                    p->addStick(s, 1);
                    sticks.push_back(s);
                }
                if (y == 0 && x % 2 == 0)
                    p->pin();

                points.push_back(p);
            }
        }
    }

    ~Cloth() {
        for (auto p : points) delete p;
        for (auto s : sticks)  delete s;
    }

    void update(float dt,
                sf::Vector2f mousePos, sf::Vector2f mousePrevPos,
                float cursorSize, bool leftDown, bool rightDown,
                int winW, int winH)
    {
        for (auto p : points)
            p->update(dt, drag, gravity, elasticity,
                      mousePos, mousePrevPos, cursorSize,
                      leftDown, rightDown, winW, winH);

        for (auto s : sticks)
            s->update();
    }

    void draw(sf::RenderWindow& window) const {
        for (auto s : sticks)
            s->draw(window);
    }
};

// ─── Mouse state ─────────────────────────────────────────────────────────────

struct Mouse {
    sf::Vector2f pos;
    sf::Vector2f prevPos;
    float cursorSize    = 20.f;
    float maxCursorSize = 100.f;
    float minCursorSize = 20.f;
    bool  leftDown  = false;
    bool  rightDown = false;

    void updatePosition(float x, float y) {
        prevPos = pos;
        pos     = { x, y };
    }

    void scroll(float delta) {
        float next = cursorSize + delta * 10.f;
        if (next >= minCursorSize && next <= maxCursorSize)
            cursorSize = next;
    }
};

// ─── main ─────────────────────────────────────────────────────────────────────

int main() {
    const int WIN_W = 1200;
    const int WIN_H = 700;

    sf::RenderWindow window(sf::VideoMode({ (unsigned)WIN_W, (unsigned)WIN_H }),
                            "Cloth Simulation");
    window.setFramerateLimit(60);

    // cloth dimensions (in grid units) and spacing
    const int CLOTH_W   = 119;   // grid columns  (1200 / 10 - 1)
    const int CLOTH_H   = 31;    // grid rows     (320  / 10 - 1)
    const int SPACING   = 10;
    const int START_X   = WIN_W / 2 - CLOTH_W * SPACING / 2;
    const int START_Y   = (int)(WIN_H * 0.05f);

    Cloth cloth(CLOTH_W, CLOTH_H, SPACING, START_X, START_Y);
    Mouse mouse;

    // cursor circle (visual aid)
    sf::CircleShape cursor(mouse.cursorSize);
    cursor.setFillColor(sf::Color(200, 200, 200, 60));
    cursor.setOutlineColor(sf::Color(100, 100, 100, 150));
    cursor.setOutlineThickness(1.f);
    cursor.setOrigin({ mouse.cursorSize, mouse.cursorSize });

    sf::Clock clock;

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())
                window.close();

            if (auto* mm = ev->getIf<sf::Event::MouseMoved>())
                mouse.updatePosition((float)mm->position.x, (float)mm->position.y);

            if (auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                mouse.updatePosition((float)mb->position.x, (float)mb->position.y);
                if (mb->button == sf::Mouse::Button::Left)  mouse.leftDown  = true;
                if (mb->button == sf::Mouse::Button::Right) mouse.rightDown = true;
            }

            if (auto* mb = ev->getIf<sf::Event::MouseButtonReleased>()) {
                if (mb->button == sf::Mouse::Button::Left)  mouse.leftDown  = false;
                if (mb->button == sf::Mouse::Button::Right) mouse.rightDown = false;
            }

            if (auto* mw = ev->getIf<sf::Event::MouseWheelScrolled>())
                mouse.scroll(mw->delta);

            if (auto* kp = ev->getIf<sf::Event::KeyPressed>())
                if (kp->code == sf::Keyboard::Key::Escape)
                    window.close();
        }

        float dt = clock.restart().asSeconds();

        cloth.update(dt,
                     mouse.pos, mouse.prevPos,
                     mouse.cursorSize,
                     mouse.leftDown, mouse.rightDown,
                     WIN_W, WIN_H);

        // update cursor visual
        cursor.setRadius(mouse.cursorSize);
        cursor.setOrigin({ mouse.cursorSize, mouse.cursorSize });
        cursor.setPosition(mouse.pos);

        window.clear(sf::Color(240, 240, 235));
        cloth.draw(window);
        window.draw(cursor);
        window.display();
    }

    return 0;
}