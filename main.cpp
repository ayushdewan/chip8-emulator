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
#define DEBUG false
#define RATE 2000.0f

#define MINCODES 0
#define MAXCODES 16

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
unsigned char dt;
unsigned char st;
unsigned short stk[16];
unsigned short sp;
unsigned char key[16];
bool drawFlag;
bool screen[32][64];

// Timer states
sf::Clock delay_clock;
sf::Clock sound_clock;

bool loadROM(string romFile)
{
    ifstream input("roms/" + romFile, ios::binary);
    if (!input)
        return false;

    vector<char> bytes(
        (istreambuf_iterator<char>(input)),
        (istreambuf_iterator<char>()));
    input.close();

    if (DEBUG)
    {
        for (int i = 1; i < (int)bytes.size(); i += 2)
        {
            unsigned int instr = ((bytes[i - 1] & 0xFF) << 8) | (bytes[i] & 0xFF);
            printf("%x: %04x\n", 0x200 + (i - 1), instr);
        }
    }

    for (int i = 0; i < (int)bytes.size(); i++)
        memory[i + 0x200] = bytes[i] & 0xFF;
    return true;
}

string initialize(int argc, char **argv)
{
    pc = 0x200;
    I = 0;
    dt = 0;
    st = 0;
    sp = 0;
    for (int i = 0; i < 80; i++)
        memory[i] = chip8_fontset[i];
    for (int i = 0; i < 16; i++)
        key[i] = 0;
    drawFlag = false;

    if (argc > 1)
    {
        char *filename = argv[1];
        string ret(filename);

        if (loadROM(ret))
            return ret;
    }

    string ret = "ibm.ch8";
    loadROM(ret);
    return ret;
}

int getKeyCode(sf::Keyboard::Key key)
{
    switch (key)
    {
    case sf::Keyboard::X:
        return 0;
    case sf::Keyboard::Num1:
        return 1;
    case sf::Keyboard::Num2:
        return 2;
    case sf::Keyboard::Num3:
        return 3;
    case sf::Keyboard::Q:
        return 4;
    case sf::Keyboard::W:
        return 5;
    case sf::Keyboard::E:
        return 6;
    case sf::Keyboard::A:
        return 7;
    case sf::Keyboard::S:
        return 8;
    case sf::Keyboard::D:
        return 9;
    case sf::Keyboard::Z:
        return 0xA;
    case sf::Keyboard::C:
        return 0xB;
    case sf::Keyboard::Num4:
        return 0xC;
    case sf::Keyboard::R:
        return 0xD;
    case sf::Keyboard::F:
        return 0xE;
    case sf::Keyboard::V:
        return 0xF;
    default:
        return -1;
    }
}

void emulateCycle()
{
    unsigned short instr = (memory[pc] << 8) | memory[pc + 1];
    unsigned short pref = instr & 0xF000;
    unsigned short x = instr & 0xF00;
    unsigned short y = instr & 0xF0;
    unsigned short n = instr & 0xF;
    unsigned short kk = instr & 0xFF;
    unsigned short nnn = instr & 0xFFF;

    if (DEBUG && instr != 0x1228)
    {
        printf("Running instruction: %04x\n", instr);
    }

    switch (pref)
    {
    case 0x0000:
    {
        if (n == 0)
        {
            for (int i = 0; i < HEIGHT; i++)
                for (int j = 0; j < WIDTH; j++)
                    screen[i][j] = false;
            drawFlag = true;
            pc += 2;
        }
        else if (n == 0xE)
        {
            pc = stk[--sp];
            pc += 2;
        }
        else
        {
            pc += 2;
        }
        break;
    }

    case 0x1000:
    {
        pc = nnn;
        break;
    }

    case 0x2000:
    {
        stk[sp++] = pc;
        pc = nnn;
        break;
    }

    case 0x3000:
    {
        if (V[x >> 8] == kk)
            pc += 2;
        pc += 2;
        break;
    }

    case 0x4000:
    {
        if (V[x >> 8] != kk)
            pc += 2;
        pc += 2;
        break;
    }

    case 0x5000:
    {
        if (V[x >> 8] == V[y >> 4])
            pc += 2;
        pc += 2;
        break;
    }

    case 0x6000:
    {
        V[x >> 8] = (unsigned char)kk;
        pc += 2;
        break;
    }

    case 0x7000:
    {
        V[x >> 8] += (unsigned char)kk;
        pc += 2;
        break;
    }

    case 0x8000:
    {
        switch (n)
        {
        case 0:
        {
            V[x >> 8] = V[y >> 4];
            break;
        }

        case 1:
        {
            V[x >> 8] |= V[y >> 4];
            break;
        }

        case 2:
        {
            V[x >> 8] &= V[y >> 4];
            break;
        }

        case 3:
        {
            V[x >> 8] ^= V[y >> 4];
            break;
        }

        case 4:
        {
            V[0xF] = (V[y >> 4] > 0xFF - V[x >> 8]);
            V[x >> 8] += V[y >> 4];
            break;
        }

        case 5:
        {
            V[0xF] = V[y >> 4] < V[x >> 8];
            V[x >> 8] -= V[y >> 4];
            break;
        }

        case 6:
        {
            V[0xF] = V[x >> 8] & 1;
            V[x >> 8] >>= 1;
            break;
        }

        case 7:
        {
            V[0xF] = V[x >> 8] < V[y >> 4];
            V[x >> 8] = V[y >> 4] - V[x >> 8];
            break;
        }

        case 0xE:
        {
            V[0xF] = V[x >> 8] & 1;
            V[x >> 8] <<= 1;
            break;
        }
        }
        pc += 2;
        break;
    }

    case 0x9000:
    {
        if (V[x >> 8] != V[y >> 4])
            pc += 2;
        pc += 2;
        break;
    }

    case 0xA000:
    {
        I = nnn;
        pc += 2;
        break;
    }

    case 0xB000:
    {
        pc = nnn + V[0];
        break;
    }

    case 0xC000:
    {
        V[x >> 8] = (rand() % 256) & kk;
        pc += 2;
        break;
    }

    case 0xD000:
    {
        V[0xF] = 0;
        drawFlag = true;
        int xval = (int)(V[x >> 8]) % 64, yval = (int)(V[y >> 4]) % 32;
        for (int l = 0; l < n; l++)
        {
            unsigned char line = memory[I + l];
            for (int k = 0; k < 8; k++)
            {
                if ((line & (0x80 >> k)) > 0)
                {
                    int cx = xval + k, cy = yval + l;
                    if (cx < WIDTH && cy < HEIGHT)
                    {
                        if (screen[cy][cx])
                            V[0xF] = 1;
                        screen[cy][cx] = !screen[cy][cx];
                    }
                }
            }
        }
        pc += 2;
        break;
    }

    case 0xE000:
    {
        if ((key[V[x >> 8]] && kk == 0x9E) || (!key[V[x >> 8]] && kk == 0xA1))
            pc += 2;
        pc += 2;
        break;
    }

    case 0xF000:
    {
        switch (kk)
        {
        case 0x07:
        {
            V[x >> 8] = dt;
            break;
        }

        case 0x0A:
        {
            bool found = false;
            for (int i = 0; i <= 0xF; i++)
            {
                if (key[i])
                {
                    V[x >> 8] = i;
                    found = true;
                }
            }

            if (!found)
            {
                pc -= 2;
            }
            break;
        }

        case 0x15:
        {
            dt = V[x >> 8];
            delay_clock.restart();
            break;
        }

        case 0x18:
        {
            st = V[x >> 8];
            sound_clock.restart();
            break;
        }

        case 0x1E:
        {
            I += V[x >> 8];
            break;
        }

        case 0x29:
        {
            I = V[x >> 8] * 5;
            break;
        }

        case 0x33:
        {
            memory[I] = V[x >> 8] / 100;
            memory[I + 1] = (V[x >> 8] / 10) % 10;
            memory[I + 2] = V[x >> 8] % 10;
            break;
        }

        case 0x55:
        {
            for (int i = 0; i <= (x >> 8); i++)
                memory[I + i] = V[i];
            break;
        }

        case 0x65:
        {
            for (int i = 0; i <= (x >> 8); i++)
                V[i] = memory[I + i];
            break;
        }
        }
        pc += 2;
        break;
    }

    default:
    {
        printf("Unknown opcode: 0x%04x\n", instr);
        exit(1);
    }
    }

    if (dt != 0 && delay_clock.getElapsedTime().asMilliseconds() >= 166)
    {
        dt--;
        delay_clock.restart();
    }

    if (st != 0 && sound_clock.getElapsedTime().asMilliseconds() >= 166)
    {
        st--;
        sound_clock.restart();
    }
}

int main(int argc, char **argv)
{
    // initialize state
    string game = initialize(argc, argv);
    sf::RenderWindow window(sf::VideoMode(64 * SCALE, 32 * SCALE), game);
    sf::Clock rate_clock;
    int cycles = 0;

    while (window.isOpen())
    {
        // emulate cycles
        int num_codes = (int)(RATE * rate_clock.getElapsedTime().asSeconds()) - cycles;
        num_codes = min(num_codes, MAXCODES);
        num_codes = max(num_codes, MINCODES);
        for (int i = 0; i < num_codes; i++)
            emulateCycle();
        cycles += num_codes;

        // draw to screen if draw flag is true
        if (drawFlag)
        {
            window.clear();
            for (int i = 0; i < HEIGHT; i++)
            {
                for (int j = 0; j < WIDTH; j++)
                {
                    if (screen[i][j])
                    {
                        sf::RectangleShape rectangle(sf::Vector2f(SCALE, SCALE));
                        rectangle.setPosition(j * SCALE, i * SCALE);
                        window.draw(rectangle);
                    }
                }
            }
            window.display();
            drawFlag = false;
        }

        // update window/key states
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed)
            {
                int keycode = getKeyCode(event.key.code);
                if (keycode != -1)
                    key[keycode] = 1;
            }

            if (event.type == sf::Event::KeyReleased)
            {
                int keycode = getKeyCode(event.key.code);
                if (keycode != -1)
                    key[keycode] = 0;
            }
        }
    }

    return 0;
}