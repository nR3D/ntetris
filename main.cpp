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

    WINDOW *mainWin = create_winbox(21, 20,  LINES/16, COLS/4);
    wrefresh(mainWin);

    WINDOW *nextWin = create_winbox(LINES/3, COLS/4, LINES/2.5, COLS - COLS/2);

    WINDOW *scoreWin = create_winbox(LINES/4, COLS/4, LINES/16, COLS - COLS/2);
    int temph, tempw;
    getmaxyx(scoreWin, temph, tempw);
    mvwprintw(scoreWin, 1, tempw/2 - 3, "Score:");
    unsigned long int scoreMatch = 0;
    update_score(scoreWin, scoreMatch);

    
    bool flag = true, frameShift = true;
    microseconds millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    milliseconds speedLevel{500};
    BRG mainGenerator{};
    tetrimino *currentBlock = get_next(mainGenerator);
    print_next(nextWin, &mainGenerator);
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
                if(frameShift)  // makes sure to avoid spamming of space_key to have infinite time before next piece is drop
                {
                    millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) + milliseconds{500} - speedLevel;
                    frameShift = false;
                }
				break;
			case 'z':
				rotateBlock(mainWin, currentBlock, 0);
				break;
			case KEY_UP:
				rotateBlock(mainWin, currentBlock, 1);
				break;
            case 'q':
                flag = !pause_game(nextWin);
                print_next(nextWin, &mainGenerator);
                break;
		}
        if(flag && (duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - millStart) > speedLevel)
        {
            if(moveBlock(mainWin, currentBlock, 1, 0))
            {
                scoreMatch += 100*check_rows(mainWin);
                update_score(scoreWin, scoreMatch);
                currentBlock = get_next(mainGenerator);
                if(moveBlock(mainWin, currentBlock, 1, 0, true))  // if next tetrimino cannot be placed then end game
                    flag = false;
                else
                {
                    print_next(nextWin, &mainGenerator);
                    millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                    frameShift = true;
                }
            }
            else if(moveBlock(mainWin, currentBlock, 1, 0, true))  // simulate movement to check if the next down translation will also be impossible
                millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) + milliseconds{500} - speedLevel;
            else
            {
                millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                frameShift = true;
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
    delwin(scoreWin);
    delwin(nextWin);
    endwin();
}
