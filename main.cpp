#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <fstream>
#include <iterator>
#include <vector>
using namespace std;

#define WIDTH 64
#define HEIGHT 32
#define SCALE 50
#define FPS 60.0f
#define DEBUG false

#define sleep_ms(t) this_thread::sleep_for(chrono::milliseconds(t))


unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// CHIP 8 STATE
unsigned char memory[4096];
unsigned char V[16];
unsigned short I;
unsigned short pc;
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short sp;
unsigned char key[16];
bool drawFlag;
bool screen[32][64];


bool loadROM(string romFile) {
    ifstream input(romFile, ios::binary);
    if(!input) return false;

    vector<char> bytes(
         (istreambuf_iterator<char>(input)),
         (istreambuf_iterator<char>()));
    input.close();

    if(DEBUG) {
        for(int i = 1; i < (int) bytes.size(); i += 2) {
            unsigned int instr =  ((bytes[i-1] & 0xFF) << 8) | (bytes[i] & 0xFF);
            printf("%04x\n", instr);
        }
    }

    for(int i = 0; i < (int) bytes.size(); i++) memory[i + 0x200] = bytes[i];
    return true;
}

string initialize(int argc, char **argv) {
    pc = 0x200;
    I = 0;
    sp = 0;
    for(int i = 0; i < 80; i++) memory[i] = chip8_fontset[i];

    if(argc > 1) {
        char* filename = argv[1];
        string ret(filename);

        if(loadROM(ret))
            return ret;
    }

    string ret = "ibm.ch8";
    loadROM(ret);
    return ret;
}

int main(int argc, char **argv)
{
    // initialize state
    string game = initialize(argc, argv);
    sf::RenderWindow window(sf::VideoMode(64 * SCALE, 32 * SCALE), game);
    window.setFramerateLimit(FPS);

    
    drawFlag = true;
    while (window.isOpen())
    {
        // emulate a cycle
        screen[4][5] = true;

        // draw to screen if draw flag is true
        if(drawFlag) {
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
        }
        
        // update window/key states
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }

    return 0;
}