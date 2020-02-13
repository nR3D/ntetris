#include "src/tetrimino.h"
#include "src/game.h"
#include "tools/debug.h"
#include <ncurses.h>
#include <chrono>
#include <cmath>

using std::chrono::duration_cast;
using std::chrono::system_clock;

WINDOW *create_winbox(int h, int w, int starty, int startx)
{
    // cannot use box() because a padding is needed
    WINDOW *local_win = newwin(h, w, starty, startx);
    mvhline(starty, startx, 0, w);
    mvvline(starty, startx-1, 0, h);
    mvhline(starty + h, startx, 0, w);
    mvvline(starty, startx + w, 0, h);
    mvaddch(starty, startx-1, ACS_ULCORNER);
    mvaddch(starty, startx+w, ACS_URCORNER);
    mvaddch(starty+h, startx-1, ACS_LLCORNER);
    mvaddch(starty+h, startx+w, ACS_LRCORNER);
    wrefresh(local_win);
    refresh();
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

    while(LINES < 23 || COLS < 37)
    {
        mvaddstr(LINES/2, COLS/2 - 11, "Please resize terminal");
        refresh();
        getmaxyx(stdscr, LINES, COLS);
        erase();
    }

    WINDOW *mainWin = create_winbox(21, 20,  1, COLS/2 - 18);
    wrefresh(mainWin);

    WINDOW *infoWin = create_winbox(7, 14, 1, COLS/2 + 4);
    int temph, tempw;
    getmaxyx(infoWin, temph, tempw);
    mvwprintw(infoWin, 1, tempw/2 - 3, "Score:");
    mvwprintw(infoWin, temph/2 + 1, tempw/2 - 3, "Level:");

    WINDOW *holdWin = create_winbox(6, 14, 9, COLS/2 + 4);
    getmaxyx(holdWin, temph, tempw);
    mvwprintw(holdWin, 1, tempw/2 - 2, "Hold");
    wrefresh(holdWin);

    WINDOW *nextWin = create_winbox(6, 14, 16, COLS/2 + 4);

    debug::wstream ws;
    Game *currentGame = new Game();
    currentGame->generator->printNext(nextWin);
    currentGame->updateInfo(infoWin);
    timeout(0);  // no wait for getch()
    while(currentGame->continueGame)
    {
        switch(getch())
	{
	    case KEY_F(1):
		currentGame->continueGame = false;
		break;
	    case KEY_LEFT:
		if(currentGame->generator->getCurrent()->getXPos() > 1 || currentGame->generator->getCurrent()->getYPos() > 0)
		    currentGame->generator->getCurrent()->move(mainWin, 0, -1);
		break;
	    case KEY_RIGHT:
		if(currentGame->generator->getCurrent()->getXPos() + 2*currentGame->generator->getCurrent()->getPivot() < 18 || currentGame->generator->getCurrent()->getYPos() > 0)
		    currentGame->generator->getCurrent()->move(mainWin, 0, 1);
		break;
	    case KEY_DOWN:
		currentGame->generator->getCurrent()->move(mainWin, 1, 0);
		break;
	    case ' ':
		while(!currentGame->generator->getCurrent()->move(mainWin, 1, 0));
		if(currentGame->lockDelay)  // makes sure to avoid spamming of space_key to have infinite time before next piece is drop
		{
		    currentGame->millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) + milliseconds{500} - currentGame->fallSpeed;
		    currentGame->lockDelay = false;
		}
		break;
	    case 'z':
		currentGame->generator->getCurrent()->rotate(mainWin, 0);
		break;
	    case KEY_UP:
		currentGame->generator->getCurrent()->rotate(mainWin, 1);
		break;
	    case 'h':
		if(currentGame->holdTurn)
		{
		    currentGame->holdTurn = false;
		    char hold_ch = currentGame->holdCh;
		    currentGame->holdCh = currentGame->generator->getCurrent()->getShape();
		    currentGame->generator->getCurrent()->erase(mainWin, true);  // erase tetrimino and refresh mainWin
		    if(hold_ch)
			currentGame->generator->swapCurrent(new tetrimino(hold_ch));
		    else
			currentGame->generator->getNext();

		    if(currentGame->holdCh)
			print_hold(holdWin, currentGame->holdCh);
			
		    currentGame->generator->printNext(nextWin);
		    currentGame->millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		    currentGame->lockDelay = true;
		}
		break;
	    case 'q':
		currentGame->continueGame = !pause(nextWin);
		if(currentGame->continueGame)
		    currentGame->generator->printNext(nextWin);
		break;
	}
	if(currentGame->continueGame && (duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - currentGame->millStart) > currentGame->fallSpeed)
	{
	    if(currentGame->generator->getCurrent()->move(mainWin, 1, 0))
	    {
		if(currentGame->generator->getCurrent()->getYPos() < 1)  // if tetrimino is blocked before entering the grid then end game
		    currentGame->continueGame = false;
		else
		{
		    currentGame->score += 100*check_rows(mainWin);
		    while(500*currentGame->level <= currentGame->score)
			++currentGame->level;
		    currentGame->updateInfo(infoWin);
		    currentGame->generator->getNext();
		    currentGame->generator->printNext(nextWin);
		    currentGame->fallSpeed = milliseconds(static_cast<int>(1000*pow(0.8-(currentGame->level-1)*0.007, currentGame->level-1)));  // formula from official tetris guidline for gravity
		    currentGame->millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		    currentGame->lockDelay = true;
		    currentGame->holdTurn = true;
		}
	    }
	    else if(currentGame->generator->getCurrent()->move(mainWin, 1, 0, true))  // simulate movement to check if the next down translation will also be impossible
		currentGame->millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) + milliseconds{500} - currentGame->fallSpeed;
	    else
	    {
		currentGame->millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		currentGame->lockDelay = true;
	    }
	}
    }

    wmove(mainWin, 1, 0);
    wclrtobot(mainWin);
    getmaxyx(mainWin, temph, tempw);
    mvwaddstr(mainWin, temph/2, tempw/2 - 5, "Game  Over");
    wrefresh(mainWin);
    timeout(-1);
    getch();
    
    delwin(mainWin);
    delwin(infoWin);
    delwin(nextWin);
    endwin();
}
