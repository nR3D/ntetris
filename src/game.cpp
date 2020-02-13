#include "game.h"

tetrimino** Game::BRG::getNewBag()
{
    tetrimino** newBag = new tetrimino*[7];
    std::random_device dev;
    std::mt19937 rng(dev());
    char name_tetr[7] = {'I','L','J','S','Z','O','T'};
    for(int i=0; i<7; i++)
    {
	std::uniform_int_distribution<std::mt19937::result_type> distBag(0,6-i);
	int rand_i = distBag(rng);
	char rand_ch = name_tetr[rand_i];
	name_tetr[rand_i] = name_tetr[6-i];
	newBag[i] = new tetrimino(rand_ch);
    }
    return newBag;
}

Game::BRG::BRG(): bag(getNewBag()), pnt(0) {}

Game::BRG::~BRG()
{
    for(int i=0; i<7; i++)
    {
	delete bag[i];
	if(nextBag)
	    delete nextBag[i];
    }
}

tetrimino* Game::BRG::getCurrent() const { return bag[pnt]; }

tetrimino* Game::BRG::getNext()
{
    /* returns next tetrimino and removes previous one */
    delete bag[pnt];
    if(++pnt > 6)
    {
	// previous pointers already deallocated
	pnt = 0;
	if(nextBag)
	{
	    bag = nextBag;
	    nextBag = nullptr;
	}
	else
	    bag = getNewBag();
    }
    return bag[pnt];
}

void Game::BRG::swapCurrent(tetrimino* t)
{
    delete bag[pnt];
    bag[pnt] = t;
}

bool Game::BRG::isEmpty() const { return pnt >= 6; }

void Game::BRG::printNext(WINDOW* win) 
{
    /* prints next tetrimino inside bag, if bag is empty a new bag is allocated*/
    int h, w;
    getmaxyx(win, h, w);

    if(isEmpty())
    {
	if(nextBag)
	{
	    for(int i = 0; i < 7; i++)
		delete nextBag[i];
	}
	nextBag = getNewBag();
    }
    tetrimino *next_tetr = new tetrimino(isEmpty() ? nextBag[0]->getShape() : bag[pnt+1]->getShape());

    wmove(win, 1, 0);
    wclrtobot(win); // erase previous terimino
    mvwaddstr(win, 1, w/2 - 2, "Next");

    next_tetr->centerPos(win);
    next_tetr->draw(win);

    wrefresh(win);
    delete next_tetr;
}

Game::Game(): generator(new BRG()), score(0), level(1), continueGame(true), lockDelay(true), holdTurn(true), holdCh('\0'), 
    fallSpeed(milliseconds(1000)), millStart(duration_cast<milliseconds>(system_clock::now().time_since_epoch())) {}

Game::~Game() { delete generator; }

void Game::updateInfo(WINDOW *win) const
{
    int len_score = 1;
    while(score/pow(10,len_score) > 1)
        len_score++;
    int h, w;
    getmaxyx(win, h, w);
    wmove(win, h/3, 0);
    wclrtoeol(win);  // remove previous score
    mvwprintw(win, h/3, w/2 - len_score/2 - 1, "%d", score);
    wmove(win, h/2 + 2, 0);
    wclrtoeol(win);  // remove previous level
    mvwprintw(win, h/2 + 2, w/2 - 1, "%d", level);
    wrefresh(win);
}

int check_rows(WINDOW *win)
{
    /* Remove completed rows and return the number or rows removed */
    
    chtype current_ch;
    int rowsRemoved = 0;
    bool single;
    for(int row=0; row<20; row++)
    {
        single = true;
        for(int c=0; c<10 && single; c++)
        {
            current_ch = mvwinch(win, 20-row, c*2);
            if(static_cast<char>(current_ch) == ' ')
                single = false;
        }
        if(single)
        {
            for(int r=row; r<20; r++)
            {
                for(int c=0; c<10; c++)
                {
                    current_ch = mvwinch(win, 19-r, c*2);
                    mvwaddch(win, 20-r, 2*c, current_ch);
                    current_ch = mvwinch(win, 19-r, 2*c+1);  // in case the second character used is not the same of the first (e.g. '[' and ']')
                    mvwaddch(win, 20-r, 2*c+1, current_ch);
                }
            }
            wrefresh(win);
            rowsRemoved++;
            row--;  // each line as been translated down by one position, so the current line must be checked again
        }
    }

    return rowsRemoved;
}

void countdown(WINDOW *win)
{
    wmove(win, 1, 0);
    wclrtobot(win);
    int h, w;
    getmaxyx(win, h, w);
    for(int i=3; i>0; i--)
    {
        mvwprintw(win, h/2, w/2-1, "%d", i);
        wrefresh(win);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    mvwaddch(win, h/2, w/2-1, ' ');
    wrefresh(win);
}

bool pause(WINDOW *win)
{
    wmove(win, 1, 0);
    wclrtobot(win);

    int h, w;
    getmaxyx(win, h, w);
    mvwprintw(win, h/2-1, w/2-3, "Pause");
    mvwprintw(win, h/2, w/2-5, "> Resume");
    mvwprintw(win, h/2+1, w/2-3, "Exit");
    wrefresh(win);
    bool flag = true;
    bool arrow_position = false;  // false: resume, true: exit
    timeout(-1);
    while(flag)
    {
        chtype ch = getch();
        if(ch == KEY_UP || ch == KEY_DOWN)
        {
            mvwaddch(win, h/2+arrow_position, w/2-5, ' ');
            arrow_position = !arrow_position;
            mvwaddch(win, h/2+arrow_position, w/2-5, '>');
            wrefresh(win);
        }
        else if(ch == 10 || ch == ' ')  // KEY_ENTER or KEY_SPACE to confirm current selection
        {
            if(!arrow_position)
                countdown(win);
            flag = false;
        }
        else if(ch == 'q')
        {
            countdown(win);
            arrow_position = false;
            flag = false;
        }
    }
    timeout(0);  // restore getch() wait time to 0
    return arrow_position;
}


void print_hold(WINDOW* win, char hold_ch)
{
    int h, w;
    getmaxyx(win, h, w);
    tetrimino *hold_tetr = new tetrimino(hold_ch);

    wmove(win, 2, 0);
    wclrtobot(win);  // erase previous tetrimino
    
    hold_tetr->centerPos(win);
    hold_tetr->draw(win);
    wrefresh(win);

    delete hold_tetr;
}
