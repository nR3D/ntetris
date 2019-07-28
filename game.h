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

#include <thread>
#include <random>

struct BRG  // Bag Random Generator
{
    tetrimino* bag[7];
    short lenBag;
    BRG()
    {
        lenBag = 7;
        std::random_device dev;
        std::mt19937 rng(dev());
        char name_tetr[7] = {'I', 'L', 'J', 'S', 'Z', 'O', 'T'};
        for(int i=0; i<7; i++)
        {
            std::uniform_int_distribution<std::mt19937::result_type> distBag(0,6-i);
            int rand_i = distBag(rng);
            char rand_ch = name_tetr[rand_i];
            if(rand_i != 6-i)
                name_tetr[rand_i] = name_tetr[6-i];
            unsigned short spawnx = 0;
            if(rand_ch == 'I' || rand_ch == 'O')
                spawnx = 8;  // tetrimino 'I' and 'O' spawn in the middle columns
            if(spawnx%2)
                spawnx /= 2; // spawn location must be a multiple of 2, otherwise tetrimino will overflow on the right corner
            bag[i] = new tetrimino(rand_ch, 0, spawnx);
        }
    }
};

tetrimino *get_next(BRG &generator)
{
    tetrimino *next_block = generator.bag[generator.lenBag-1];
    generator.lenBag--;
    if(!generator.lenBag)
        generator = BRG();
    return next_block;
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

void update_score(WINDOW *win, unsigned long int score)
{
    int len_score = 0;
    while(score/pow(10,len_score) > 1)
        len_score++;
    if(len_score%2)
        len_score++;
    int h, w;
    getmaxyx(win, h, w);
    mvwprintw(win, h/2, w/2 - len_score/2, "%d", score);
    wrefresh(win);
}

void count_down(WINDOW *win)
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

bool pause_game(WINDOW *win)
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
        else if(ch == 10)  // KEY_ENTER
        {
            if(!arrow_position)
                count_down(win);
            flag = false;
        }
        else if(ch == 'q')
        {
            count_down(win);
            arrow_position = false;
            flag = false;
        }
    }
    timeout(0);  // restore getch() wait time to 0
    return arrow_position;
}

void print_next(WINDOW *win, BRG *generator)
{
    // The next tetrimino in *generator will be printed at the centre of *win
    int h, w;
    getmaxyx(win, h, w);
    tetrimino *next_tetr = new tetrimino(generator->bag[generator->lenBag-1]->shape, h/2, w/2 - 2);

    wmove(win, 1, 0);
    wclrtobot(win);  // erase previous tetrimino
    
    mvwaddstr(win, 1, w/2 - 2, "Next");
    drawBlock(win, next_tetr);
    wrefresh(win);
}
