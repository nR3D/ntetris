#ifndef NCURSES_H
#define NCURSES_H
#include <ncurses.h>
#endif

struct tetrimino
{
	char shape;
	int yPos;
	int xPos;
	int (*coord)[2];
	int lenCoord;  // for classic tetriminos it's always 4, but can be used to create custom shapes
	int pivot;  // define square where the tetrimino is rotated
	short colorPair;
	tetrimino(char ch, int y, int x)
	{
		shape = ch;
		yPos = y;
		xPos = x;
		short f, bgcolor;
		pair_content(1, &f, &bgcolor);  // get background color from COLOR_PAIR(1)
		switch(shape)
		{
			case 'I':
			    coord = new int[4][2]{{1,0},{1,1},{1,2},{1,3}};
			    lenCoord = 4;
			    init_pair(2, COLOR_CYAN, bgcolor);
			    colorPair = 2;
				pivot = 3;
			    break;
			case 'L':
				coord = new int[4][2]{{1,0},{1,1},{1,2},{0,2}};
				lenCoord = 4;
				if(can_change_color())  // try changing color to orange, otherwise assign a white color
				{
					init_color(8, 1000,600,0);
					init_pair(3, 8, bgcolor);
				}
				else
					init_pair(3, COLOR_WHITE, bgcolor);
				colorPair = 3;
				pivot = 2;
				break;
			case 'J':
				coord = new int[4][2]{{0,0},{1,0},{1,1},{1,2}};
				lenCoord = 4;
				init_pair(4, COLOR_BLUE, bgcolor);
				colorPair = 4;
				pivot = 2;
				break;
			case 'O':
				coord = new int[4][2]{{0,0},{0,1},{1,0},{1,1}};
				lenCoord = 4;
				init_pair(5, COLOR_YELLOW, bgcolor);
				colorPair = 5;
				pivot = 1;
				break;
			case 'S':
				coord = new int[4][2]{{0,1},{0,2},{1,0},{1,1}};
				lenCoord = 4;
				init_pair(6, COLOR_GREEN, bgcolor);
				colorPair = 6;
				pivot = 2;
				break;
			case 'Z':
				coord = new int[4][2]{{0,0},{0,1},{1,1},{1,2}};
				lenCoord = 4;
				init_pair(7, COLOR_RED, bgcolor);
				colorPair = 7;
				pivot = 2;
				break;
			case 'T':
				coord = new int[4][2]{{1,0},{1,1},{1,2},{0,1}};
				lenCoord = 4;
				init_pair(8, COLOR_MAGENTA, bgcolor);
				colorPair = 8;
				pivot = 2;
				break;
		}
	}
};

void drawBlock(WINDOW *win, tetrimino *block)
{
	wcolor_set(win,block->colorPair,0);
	chtype ch;
	for(int i = 0; i<block->lenCoord; i++)
	{
		ch = mvwinch(win, block->coord[i][0] + block->yPos, 2*block->coord[i][1] + block->xPos);
		if((static_cast<char>(ch) == ' ') && (block->coord[i][0] + block->yPos >= 1))
		{
			mvwaddch(win, block->coord[i][0] + block->yPos, 2*block->coord[i][1] + block->xPos, ACS_CKBOARD);
			mvwaddch(win, block->coord[i][0] + block->yPos, 2*block->coord[i][1] + block->xPos + 1, ACS_CKBOARD);
		}
	}
	wcolor_set(win,1,0);
	wrefresh(win);
}

bool moveBlock(WINDOW *win, tetrimino *block, short diry, short dirx, bool simulate_movement=false)
{
	/* Move tetrimino by (diry, dirx) positions, dirx: -1 left and 1 right, diry: -1 up and 1 down.
	 * It's meant to be a small movement in a 10x20 grid, that's why the variables are short and not int,
	 * but changing them to ints, if needed, shouldn't break the code, by the way keep in mind that
	 * move() functions are defined with integers and so larger types should not be used.
	 *
	 * Return TRUE if there wasn't enough space to move, otherwise FALSE
	 */

	for(int i=0; i<block->lenCoord; i++)  // remove current block position
		if(block->coord[i][0] + block->yPos >= 1)
			mvwaddstr(win, block->coord[i][0] + block->yPos, 2*block->coord[i][1] + block->xPos, "  ");
	
	bool next_free = true;
	chtype next_ch;
	for(int i=0; i<block->lenCoord; i++)  // check if the new position can be drawn without overlaps with other blocks
	{
		next_ch = mvwinch(win, block->coord[i][0] + block->yPos + diry, 2*block->coord[i][1] + block->xPos + 2*dirx);
		next_free = next_free && static_cast<char>(next_ch) == ' ';
	}
	if(next_free && !simulate_movement)  // if new position is free then translate the block and draw it, otherwise draw the previous position
	{
		block->yPos += diry;
		block->xPos += 2*dirx;
	}
	drawBlock(win, block);
	return !next_free;
}

void rotateBlock(WINDOW *win, tetrimino *block, bool angle)  // angle: 0 (90°, rotate left), 1 (-90°, rotate right)
{
	for(int i=0; i<block->lenCoord; i++)  // remove current block position
		if(block->coord[i][0] + block->yPos >= 1)
			mvwaddstr(win, block->coord[i][0] + block->yPos, 2*block->coord[i][1] + block->xPos, "  ");
	
	// reverse y-x coordinates
	int temp;
	for(int i=0; i < block->lenCoord; i++)
	{
		temp = block->coord[i][0];
		block->coord[i][0] = block->coord[i][1];
		block->coord[i][1] = temp;
	}
	
	// flip block verticaly (angle==0) or horizontaly (angle=1)
	for(int i=0; i < block->lenCoord; i++)
		block->coord[i][angle] = block->pivot - block->coord[i][angle];
	
	// check if the rotated position can be drawn without overlaps with other blocks
	bool block_free = true;
	chtype ch;
	for(int i=0; i<block->lenCoord; i++)
	{
		ch = mvwinch(win, block->coord[i][0] + block->yPos, 2*block->coord[i][1] + block->xPos);
		block_free = block_free && static_cast<char>(ch) == ' ';
	}
	
	if(!block_free)  // reverse rotation if there is not space
	{
		for(int i=0; i < block->lenCoord; i++) // reverse flip
			block->coord[i][angle] = block->pivot - block->coord[i][angle];
			
		for(int i=0; i < block->lenCoord; i++) // reverse coord swap
		{
			temp = block->coord[i][0];
			block->coord[i][0] = block->coord[i][1];
			block->coord[i][1] = temp;
		}
	}
	
	drawBlock(win, block);
}
