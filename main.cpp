#ifndef NCURSES_H
#define NCURSES_H
#include <ncurses.h>
#endif

#ifndef TETRIMINO_H
#define TETRIMINO_H
#include "tetrimino.h"
#endif

#include "game.h"
#include <cmath>
#include <chrono>

using namespace std::chrono;

int main()
{
    initscr();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    attron(COLOR_PAIR(1));

    cbreak();
    noecho();
    keypad(stdscr, true);
    curs_set(0);  // cursor not visible
    refresh();

    WINDOW *mainWin = create_winbox(LINES - LINES/5, COLS/3,  LINES/10, COLS/6);
    wrefresh(mainWin);

    int temph, tempw;

    WINDOW *scoreWin = create_winbox(LINES/4, COLS/4, LINES/10, COLS - COLS/2.5);
    wattron(scoreWin, COLOR_PAIR(1));
    getmaxyx(scoreWin, temph, tempw);
    mvwprintw(scoreWin, 1, tempw/2 - 3, "Score:");
    update_score(scoreWin, 99999);

    WINDOW *nextWin = create_winbox(LINES/3, COLS/4, LINES/3, COLS - COLS/2.5);
    wattron(nextWin, COLOR_PAIR(1));
    getmaxyx(nextWin, temph, tempw);
    mvwprintw(nextWin, 1, tempw/2 - 2, "Next");
    wrefresh(nextWin);

    bool flag = true;
    microseconds millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds speedLevel{999};
    BRG mainGenerator{};
    tetrimino *currentBlock = get_next(mainGenerator);
    timeout(0);  // no wait for getch()
    while(flag)
    {
        switch(getch())
		{
			case KEY_F(1):
				flag = false;
				break;
			case KEY_LEFT:
				moveBlock(mainWin, currentBlock, 0, -1);
				break;
			case KEY_RIGHT:
				moveBlock(mainWin, currentBlock, 0, 1);
				break;
			case KEY_DOWN:
				moveBlock(mainWin, currentBlock, 1, 0);
				break;
			case ' ':
				while(!moveBlock(mainWin, currentBlock, 1, 0));
				break;
			case 'z':
				rotateBlock(mainWin, currentBlock, 0);
				break;
			case KEY_UP:
				rotateBlock(mainWin, currentBlock, 1);
				break;
		}
        if((duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - millStart) > speedLevel)
        {
            if(moveBlock(mainWin, currentBlock, 1, 0))
                currentBlock = get_next(mainGenerator);
            millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        }
    }
    
    delwin(mainWin);
    delwin(scoreWin);
    delwin(nextWin);
    endwin();
}
