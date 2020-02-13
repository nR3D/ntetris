#pragma once

#include <ncurses.h>

namespace debug
{
    class wstream
    {
	private:
	    WINDOW* win;
	    int yCursor;
	    static int H_SIZE;
	    static int W_SIZE;
	    static int Y_START;
	    static int X_START;
	    void scrollText()
	    {
		chtype current_ch;
		for(int row=2; row < H_SIZE-1; row++)
		{
		    for(int ich=1; ich < W_SIZE-2; ich++)
		    {
			current_ch = mvwinch(win, row, ich);
			mvwaddch(win, row-1, ich, current_ch);
		    }
		}
		char* blankLine = new char[W_SIZE-2];
		for(int i=0; i < W_SIZE-2; i++) blankLine[i] = ' ';
		--yCursor;
		write(blankLine);
		--yCursor;
		delete blankLine;
	    }
	public:
	    wstream(): win(newwin(H_SIZE, W_SIZE, Y_START, X_START)), yCursor(1) { box(win,0,0); }
	    ~wstream() { delwin(win); }
	    void write(int m) 
	    { 
		if(yCursor >= H_SIZE-1) scrollText();
		mvwprintw(win, yCursor++, 1, "%d", m); 
		wrefresh(win);
	    }
	    void write(const char* m) 
	    { 
		if(yCursor >= H_SIZE-1) scrollText();
		mvwprintw(win, yCursor++, 1, "%s", m); 
		wrefresh(win);
	    }
	    void write(const unsigned char* m)
	    { 
		if(yCursor >= H_SIZE-1) scrollText();
		mvwprintw(win, yCursor++, 1, "%u", m); 
		wrefresh(win);
	    }
    };
    int wstream::H_SIZE = 20;
    int wstream::W_SIZE = 15;
    int wstream::Y_START = 1;
    int wstream::X_START = 1;
}

debug::wstream& operator<<(debug::wstream& ws, int m)
{
    ws.write(m);
    return ws;
}

debug::wstream& operator<<(debug::wstream& ws, const char* m)
{
    ws.write(m);
    return ws;
}

debug::wstream& operator<<(debug::wstream& ws, void* pm)
{
    unsigned char* m = reinterpret_cast<unsigned char*>(pm);
    ws.write(m);
    return ws;
}
