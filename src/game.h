#pragma once

#include <ncurses.h>
#include <chrono>
#include <thread>
#include <random>
#include "tetrimino.h"

using std::chrono::duration_cast;
using std::chrono::system_clock;
using std::chrono::milliseconds;

struct Game
{
    public:
	class BRG // Bag Random Generator
	{
	    private:
		tetrimino** bag;
		tetrimino** nextBag;
		short pnt;
		static tetrimino** getNewBag(); 
	    public:
		BRG();
		~BRG();
		tetrimino* getCurrent() const;
		tetrimino* getNext();
		void swapCurrent(tetrimino*);
		bool isEmpty() const;
		void printNext(WINDOW*);
	} *generator;
	unsigned long int score;
	short level;
	bool continueGame; // match ends if false
	bool lockDelay; // more time can be given when a tetrimino touches the end of a column
	bool holdTurn; // can hold a tetrimino
	char holdCh; // char shape of the held tetrimino
	milliseconds millStart;
	milliseconds fallSpeed;
	Game();
	~Game();
	void updateInfo(WINDOW*) const;
};

int check_rows(WINDOW*);

void countdown(WINDOW*);

bool pause(WINDOW*);

void print_hold(WINDOW*, char);
