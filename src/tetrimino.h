#pragma once

#include <ncurses.h>

class tetrimino
{
    private:
	char shape;
	short yPos; // 10x20 grid
	short xPos;
	short (*coord)[2];
	short lenCoord; // fixed to 4 for classic tetriminos, but can be used for custom ones
	short pivot; // rotation pivot, each tetrimino rotates inside a square of size pivot
	short colorPair;
    public:
	tetrimino(char, int y=-1, int x=0);
	void erase(WINDOW*, bool refrash=false) const;
	void draw(WINDOW*) const;
	bool move(WINDOW*, short diry, short dirx, bool simulate_movement=false);
	void centerPos(WINDOW*);
	void rotate(WINDOW*, bool angle);
	char getShape() const;
	short getYPos() const;
	short getXPos() const;
	short getPivot() const;
};
