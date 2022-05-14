#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;

#define WIDTH 64
#define HEIGHT 32
#define SCALE 50
#define FPS 1.0f

#define sleep_ms(t) this_thread::sleep_for(chrono::milliseconds(t))

bool screen[32][64];

int main()
{
    sf::RenderWindow window(sf::VideoMode(64 * SCALE, 32 * SCALE), "SFML works!");
    window.setFramerateLimit(FPS);

    
    int x = 0, y = 0;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        screen[y][x] = true;

        window.clear();
        for(int i = 0; i < 32; i++) {
            for(int j = 0; j < 64; j++) {
                if(screen[i][j]) {
                    sf::RectangleShape rectangle(sf::Vector2f(SCALE, SCALE));
                    rectangle.setPosition(j * SCALE, i * SCALE);
                    window.draw(rectangle);
                }
            }
        }

        window.display();
        screen[y][x] = false;
        x++, y++;
        x %= 64;
        y %= 32;
        
    }

    return 0;
}