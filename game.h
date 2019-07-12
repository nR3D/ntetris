#ifndef NCURSES_H
#define NCURSES_H
#include <ncurses.h>
#endif

#ifndef TETRIMINO_H
#define TETRIMINO_H
#include "tetrimino.h"
#endif

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
            unsigned short spawnx = 2;
            if(name_tetr[rand_i] == 'I' || name_tetr[rand_i] == 'O')
                spawnx = 6;  // tetrimino 'I' and 'O' spawn in the middle columns
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