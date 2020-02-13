#include "tetrimino.h"

tetrimino::tetrimino(char ch, int y, int x): shape(ch), yPos(y)
{
    if(yPos == -1)
    {
	if(shape == 'O')
	    xPos = 8;
	else if (shape == 'I')
	    xPos = 6;
	else
	    xPos = 0;
    }
    short f, bgcolor;
    pair_content(1, &f, &bgcolor);  // get background color from COLOR_PAIR(1)
    switch(shape)
    {
	case 'I':
	    coord = new short[4][2]{{1,0},{1,1},{1,2},{1,3}};
	    lenCoord = 4;
	    init_pair(2, COLOR_CYAN, bgcolor);
	    colorPair = 2;
	    break;
	case 'L':
	    coord = new short[4][2]{{1,0},{1,1},{1,2},{0,2}};
	    lenCoord = 4;
	    if(can_change_color())  // try changing color to orange, otherwise assign a white color
	    {
		init_color(8, 1000,600,0);
		init_pair(3, 8, bgcolor);
	    }
	    else
		init_pair(3, COLOR_WHITE, bgcolor);
	    colorPair = 3;
	    break;
	case 'J':
	    coord = new short[4][2]{{0,0},{1,0},{1,1},{1,2}};
	    lenCoord = 4;
	    init_pair(4, COLOR_BLUE, bgcolor);
	    colorPair = 4;
	    break;
	case 'O':
	    coord = new short[4][2]{{0,0},{0,1},{1,0},{1,1}};
	    lenCoord = 4;
	    init_pair(5, COLOR_YELLOW, bgcolor);
	    colorPair = 5;
	    break;
	case 'S':
	    coord = new short[4][2]{{0,1},{0,2},{1,0},{1,1}};
	    lenCoord = 4;
	    init_pair(6, COLOR_GREEN, bgcolor);
	    colorPair = 6;
	    break;
	case 'Z':
	    coord = new short[4][2]{{0,0},{0,1},{1,1},{1,2}};
	    lenCoord = 4;
	    init_pair(7, COLOR_RED, bgcolor);
	    colorPair = 7;
	    break;
	case 'T':
	    coord = new short[4][2]{{1,0},{1,1},{1,2},{0,1}};
	    lenCoord = 4;
	    init_pair(8, COLOR_MAGENTA, bgcolor);
	    colorPair = 8;
	    break;
    }
	
    for(int i = 0; i < lenCoord; i++)
	if(coord[i][1] > pivot)
	    pivot = coord[i][1];
}

void tetrimino::erase(WINDOW *win, bool refresh) const
{
    for(int i=0; i < lenCoord; i++)
	if(coord[i][0] + yPos > 0)
	    mvwaddstr(win, coord[i][0] + yPos, 2*coord[i][1] + xPos, "  ");

    if(refresh)
	wrefresh(win);
}

void tetrimino::draw(WINDOW* win) const
{
    wcolor_set(win,colorPair,0);
    chtype ch;
    for(int i = 0; i < lenCoord; i++)
    {
	ch = mvwinch(win, coord[i][0] + yPos, 2*coord[i][1] + xPos);
	if((static_cast<char>(ch) == ' ') && (coord[i][0] + yPos > 0))
	{
	    mvwaddch(win, coord[i][0] + yPos, 2*coord[i][1] + xPos, ACS_CKBOARD);
	    mvwaddch(win, coord[i][0] + yPos, 2*coord[i][1] + xPos + 1, ACS_CKBOARD);
	}
    }
    wcolor_set(win,1,0);
    wrefresh(win);
}

bool tetrimino::move(WINDOW* win, short diry, short dirx, bool simulate_movement)
{
    /* Moves tetrimino by (diry, dirx) positions, dirx: -1 left and 1 right, diry: -1 up and 1 down.
     * Returns true if there's not enough space to move the object, otherwise false.
     * simulate_movement can be used to test a movement and get the corresponding return without applying
     * said movement.
     */
    
    erase(win);  // erases tetrimino from screen
    
    bool next_free = true;
    chtype next_ch;
    for(int i=0; i < lenCoord && next_free; i++)  // check if the new position can be drawn without overlaps with other blocks
    {
	    if(coord[i][0] + yPos + diry > 0)
	    {
		    next_ch = mvwinch(win, coord[i][0] + yPos + diry, 2*coord[i][1] + xPos + 2*dirx);
		    next_free = next_free && static_cast<char>(next_ch) == ' ';
	    }
    }
    if(next_free && !simulate_movement)  /* if new position is free and it's not a simulation then translate
									      * the block and draw it, otherwise draw the previous position */
    {
	    yPos += diry;
	    xPos += 2*dirx;
    }
    draw(win);
    return !next_free;
}

void tetrimino::centerPos(WINDOW* win)
{
    int h, w;
    getmaxyx(win, h, w);
    yPos = h/2;
    xPos = w/2 - pivot - 1;
}

void tetrimino::rotate(WINDOW* win, bool angle)
{
    // angle: 0 (90°, rotate left), 1 (-90°, rotate right)

    erase(win);

    // reverse y-x coordinates
    int temp;
    for(int i=0; i < lenCoord; i++)
    {
	temp = coord[i][0];
	coord[i][0] = coord[i][1];
	coord[i][1] = temp;
    }
    
    // flip block verticaly (angle==0) or horizontaly (angle=1)
    for(int i=0; i < lenCoord; i++)
	coord[i][angle] = pivot - coord[i][angle];
    
    // check if the rotated position can be drawn without overlaps with other blocks
    bool block_free = true;
    chtype ch;
    for(int i=0; i<lenCoord; i++)
    {
	ch = mvwinch(win, coord[i][0] + yPos, 2*coord[i][1] + xPos);
	block_free = block_free && static_cast<char>(ch) == ' ';
    }
    
    if(!block_free)  // reverse rotation if there is not enough space
    {
	for(int i=0; i < lenCoord; i++) // reverse flip
	    coord[i][angle] = pivot - coord[i][angle];
		
	for(int i=0; i < lenCoord; i++) // reverse coord swap
	{
	    temp = coord[i][0];
	    coord[i][0] = coord[i][1];
	    coord[i][1] = temp;
	}
    }
    draw(win);
}

char tetrimino::getShape() const { return shape; }

short tetrimino::getYPos() const { return yPos; }

short tetrimino::getXPos() const { return xPos; }

short tetrimino::getPivot() const { return pivot; }
