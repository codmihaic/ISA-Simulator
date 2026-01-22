#include "isa.h"
#include "parse_code.h"
#include <ncurses.h>

int main() 
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int height = LINES - 2;
    int width = COLS / 2 - 2;
    WINDOW *win1, *win2;

    init_windows(&win1, &win2, height, width);

    Buffer *buffer = create_buffer();
    handle_input(buffer, win1, win2);

    free_buffer(buffer);
    endwin();

    return 0;
}
