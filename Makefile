all:
	g++ main.cpp -I/opt/homebrew/Cellar/sfml/2.5.1_1/include/ -L/opt/homebrew/Cellar/sfml/2.5.1_1/lib/ -lsfml-graphics -lsfml-window -lsfml-system

clean:
	rm a.out