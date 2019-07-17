#ifndef NCURSES_H
#define NCURSES_H
#include <ncurses.h>
#endif

#ifndef TETRIMINO_H
#define TETRIMINO_H
#include "tetrimino.h"
#endif

#ifndef CHRONO
#define CHORNO
#include <chrono>
#endif

#include "game.h"
#include <cmath>

using namespace std::chrono;

WINDOW *create_winbox(int h, int w, int starty, int startx)
{
    WINDOW *local_win = newwin(h, w, starty, startx);
    mvhline(starty, startx, 0, w);
    mvvline(starty, startx-1, 0, h);
    mvhline(starty + h, startx, 0, w);
    mvvline(starty, startx + w, 0, h);
    mvaddch(starty, startx-1, ACS_ULCORNER);
    mvaddch(starty, startx+w, ACS_URCORNER);
    mvaddch(starty+h, startx-1, ACS_LLCORNER);
    mvaddch(starty+h, startx+w, ACS_LRCORNER);
    //box(local_win,0,0);
    return local_win;
}

int main()
{
    initscr();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    attron(COLOR_PAIR(1));

    cbreak();
    noecho();
    keypad(stdscr, true);
    curs_set(0);  // cursor not visible
    refresh();

    //WINDOW *mainWin = create_winbox(LINES - LINES/5, COLS/3,  LINES/10, COLS/6);
    WINDOW *mainWin = create_winbox(21, 20,  LINES/16, COLS/4);
    wrefresh(mainWin);

    int temph, tempw;

    WINDOW *scoreWin = create_winbox(LINES/4, COLS/4, LINES/16, COLS - COLS/2);
    wattron(scoreWin, COLOR_PAIR(1));
    getmaxyx(scoreWin, temph, tempw);
    mvwprintw(scoreWin, 1, tempw/2 - 3, "Score:");
    update_score(scoreWin, 99999);

    WINDOW *nextWin = create_winbox(LINES/3, COLS/4, LINES/2.5, COLS - COLS/2);
    wattron(nextWin, COLOR_PAIR(1));
    getmaxyx(nextWin, temph, tempw);
    mvwprintw(nextWin, 1, tempw/2 - 2, "Next");
    wrefresh(nextWin);

    bool flag = true;
    microseconds millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds speedLevel{500};
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
            case 'q':
                flag = !pause_game(nextWin);
                break;
		}
        if(flag && (duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - millStart) > speedLevel)
        {
            if(moveBlock(mainWin, currentBlock, 1, 0))
            {
                check_rows(mainWin);
                currentBlock = get_next(mainGenerator);
            }
            millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        }
    }
    
    delwin(mainWin);
    delwin(scoreWin);
    delwin(nextWin);
    endwin();
}
