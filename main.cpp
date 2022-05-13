#include <iostream>
#include <cstdlib>
#include <chrono>
#include <thread>
using namespace std;

#define WIDTH 64
#define HEIGHT 32

bool screen[HEIGHT][WIDTH];

void drawLine() {
    cout << '+';
    for(int j = 0; j < WIDTH; j++) cout << '-';
    cout << '+' << endl;
}

void drawScreen() {
    drawLine();
    for(int i = 0; i < HEIGHT; i++) {
        cout << '|';
        for(int j = 0; j < WIDTH; j++) {
            if(screen[i][j]) cout << "█";
            else cout << ' ';
        }
        cout << '|' << endl;
    }
    drawLine();
}

int main() {
    int x = 0, y = 0;
    while(x < WIDTH && y < HEIGHT) {
        system("clear");
        screen[y][x] = true;
        drawScreen();
        screen[y][x] = false;
        x++;
        y++;
        this_thread::sleep_for(chrono::milliseconds(50));
    }
    return 0;
}