CC=g++

main: main.cpp tetrimino game
	$(CC) main.cpp .out/game.o .out/tetrimino.o -lncurses -o ntetris.o

game: src/game.h src/game.cpp 
	$(CC) -c src/game.cpp -o .out/game.o

tetrimino: src/tetrimino.h src/tetrimino.cpp
	$(CC) -c src/tetrimino.cpp -o .out/tetrimino.o

clear:
	rm .out/game.o .out/tetrimino.o
