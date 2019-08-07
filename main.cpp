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

    WINDOW *mainWin = create_winbox(21, 20,  LINES/16, COLS/3);
    wrefresh(mainWin);

    WINDOW *scoreWin = create_winbox(LINES/3.3, COLS/6, LINES/16, COLS - COLS/2);
    int temph, tempw;
    getmaxyx(scoreWin, temph, tempw);
    mvwprintw(scoreWin, 1, tempw/2 - 2, "Score:");
    mvwprintw(scoreWin, temph/2 + 1, tempw/2 - 2, "Level:");

    WINDOW *holdWin = create_winbox(LINES/4, COLS/6, LINES/2.5, COLS - COLS/2);
    getmaxyx(holdWin, temph, tempw);
    mvwprintw(holdWin, 1, tempw/2 - 2, "Hold");
    wrefresh(holdWin);

    WINDOW *nextWin = create_winbox(LINES/4, COLS/6, LINES/1.5, COLS - COLS/2);

    
    Game currentGame{};
    print_next(nextWin, currentGame.generator);
    update_info(scoreWin, currentGame.score, currentGame.level);
    timeout(0);  // no wait for getch()
    while(currentGame.continueGame)
    {
        switch(getch())
		{
			case KEY_F(1):
				currentGame.continueGame = false;
				break;
			case KEY_LEFT:
                if(currentGame.currentBlock->xPos > 1 || currentGame.currentBlock->yPos > 0)
				    moveBlock(mainWin, currentGame.currentBlock, 0, -1);
				break;
			case KEY_RIGHT:
                if(currentGame.currentBlock->xPos + 2*currentGame.currentBlock->pivot < 18 || currentGame.currentBlock->yPos > 0)
				    moveBlock(mainWin, currentGame.currentBlock, 0, 1);
				break;
			case KEY_DOWN:
				moveBlock(mainWin, currentGame.currentBlock, 1, 0);
				break;
			case ' ':
				while(!moveBlock(mainWin, currentGame.currentBlock, 1, 0));
                if(currentGame.lockDelay)  // makes sure to avoid spamming of space_key to have infinite time before next piece is drop
                {
                    currentGame.millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) + milliseconds{500} - currentGame.fallSpeed;
                    currentGame.lockDelay = false;
                }
				break;
			case 'z':
				rotateBlock(mainWin, currentGame.currentBlock, 0);
				break;
			case KEY_UP:
				rotateBlock(mainWin, currentGame.currentBlock, 1);
				break;
            case 'h':
                if(currentGame.holdTurn)
                {
                    currentGame.holdTurn = false;
                    char hold_ch = currentGame.holdCh;
                    currentGame.holdCh = currentGame.currentBlock->shape;
                    erase_tetrimino(mainWin, currentGame.currentBlock, true);  // erase tetrimino and refresh mainWin
                    if(hold_ch)
                        currentGame.currentBlock = spawn_tetrimino(hold_ch);
                    else
                        currentGame.currentBlock = get_next(currentGame.generator);

                    if(currentGame.holdCh)
                        print_hold(holdWin, currentGame.holdCh);
                    
                    print_next(nextWin, currentGame.generator);
                    currentGame.millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                    currentGame.lockDelay = true;
                }
                break;
            case 'q':
                currentGame.continueGame = !pause_game(nextWin);
                print_next(nextWin, currentGame.generator);
                break;
		}
        if(currentGame.continueGame && (duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - currentGame.millStart) > currentGame.fallSpeed)
        {
            if(moveBlock(mainWin, currentGame.currentBlock, 1, 0))
            {
                if(currentGame.currentBlock->yPos < 1)  // if tetrimino is blocked before entering the grid then end game
                    currentGame.continueGame = false;
                else
                {
                    currentGame.score += 100*check_rows(mainWin);
                    while(500*currentGame.level <= currentGame.score)
                        ++currentGame.level;
                    update_info(scoreWin, currentGame.score, currentGame.level);
                    currentGame.currentBlock = get_next(currentGame.generator);
                    print_next(nextWin, currentGame.generator);
                    currentGame.fallSpeed = milliseconds(static_cast<int>(1000*pow(0.8-(currentGame.level-1)*0.007, currentGame.level-1)));  // formula from official tetris guidline for gravity
                    currentGame.millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                    currentGame.lockDelay = true;
                    currentGame.holdTurn = true;
                }
            }
            else if(moveBlock(mainWin, currentGame.currentBlock, 1, 0, true))  // simulate movement to check if the next down translation will also be impossible
                currentGame.millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) + milliseconds{500} - currentGame.fallSpeed;
            else
            {
                currentGame.millStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
                currentGame.lockDelay = true;
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
