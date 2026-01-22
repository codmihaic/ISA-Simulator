#include "isa.h"

void start_ncurses()
{    
    if (initscr() == NULL) 
    {
        fprintf(stderr, "Eroare: Nu s-a putut inițializa ncurses.\n");
        exit(-1);
    }
    cbreak(); 
    noecho(); 
    keypad(stdscr, TRUE);
    start_color(); 
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    bkgd(COLOR_PAIR(1));
    refresh();
}

void end_ncurses(WINDOW *menuwin_left, WINDOW *menuwin_right)
{
    werase(menuwin_right);
    wrefresh(menuwin_right);
    delwin(menuwin_right);

    werase(menuwin_left);
    wrefresh(menuwin_left);
    delwin(menuwin_left);
    endwin();
}

Buffer *create_buffer() 
{
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    buffer->lines = (char **)malloc(MAX_LINII * sizeof(char *));
    for (int i = 0; i < MAX_LINII; i++) 
    {
        buffer->lines[i] = (char *)calloc(MAX_COLS, sizeof(char));
    }
    buffer->num_lines = MAX_LINII;
    return buffer;
}

void insert_char(char *line, int pos, char ch, int max_cols) 
{
    if (pos < max_cols - 1) 
    {
        memmove(line + pos + 1, line + pos, max_cols - pos - 1);
        line[pos] = ch;
    }
}

void delete_char(char *line, int pos, int max_cols) 
{
    if (pos < max_cols - 1) 
    {
        memmove(line + pos, line + pos + 1, max_cols - pos - 1);
        line[max_cols - 1] = ' ';
    }
}

void insert_new_line(Buffer *buffer, int line, int max_cols) 
{
    if (line >= buffer->num_lines) 
    {
        buffer->lines = (char **)realloc(buffer->lines, (line + 1) * sizeof(char *));
        for (int i = buffer->num_lines; i <= line; i++) 
        {
            buffer->lines[i] = (char *)calloc(max_cols, sizeof(char));
        }
        buffer->num_lines = line + 1;
    }
}

void free_buffer(Buffer *buffer) 
{
    for (int i = 0; i < buffer->num_lines; i++) 
    {
        free(buffer->lines[i]);
    }
    free(buffer->lines);
    free(buffer);
}

void display_message(WINDOW *win, const char *format, ...) 
{
    va_list args;
    va_start(args, format);
    static int row = 1;
    if (row >= LINES - 2) 
    {
        werase(win);
        box(win, 0, 0);
        row = 1;
    }

    wmove(win, row, 1);    // cursorul pe coloana 1 a randului
    vw_printw(win, format, args);    // afisare mesaj
    wrefresh(win);
    wgetch(win);
    row++;
    va_end(args);
}

void handle_error(WINDOW *win, const char *format, ...) 
{
    va_list args;
    va_start(args, format);

    int height, width;
    getmaxyx(win, height, width);
    wmove(win, height - 2, 1); 
    wclrtoeol(win);
    wattron(win, A_BOLD | COLOR_PAIR(1)); 
    vw_printw(win, format, args);  
    wattroff(win, A_BOLD | COLOR_PAIR(1));
    
    wrefresh(win);
    va_end(args);
    wgetch(win);
}
void init_windows(WINDOW **win1, WINDOW **win2, int height, int width) 
{
    *win1 = newwin(height, width, 0, 0);
    *win2 = newwin(height, width, 0, width + 2);
    box(*win1, 0, 0);
    box(*win2, 0, 0);
    keypad(*win1, TRUE);
    keypad(*win2, TRUE);
    wrefresh(*win1);
    wrefresh(*win2);
}

void handle_output_window(Buffer *buffer, WINDOW *win2) 
{
    werase(win2);
    box(win2, 0, 0);
    wmove(win2, 1, 1);
    parse_lines(buffer->lines, buffer->num_lines, win1, win2);
    wrefresh(win2);
    // wgetch(win2);
}

//  intrarea utilizatorului
void handle_input(Buffer *buffer, WINDOW *win1, WINDOW *win2) 
{
    int row = 0, col = 0;
    int scroll_offset = 0;
    int ch;

    while ((ch = wgetch(win1)) != KEY_F(5)) 
    {
        switch (ch) 
        {
            case '\n': 
            {
                insert_new_line(buffer, row + scroll_offset + 1, MAX_COLS);
                // mutam textul pe lini urmatoare
                char *current_line = buffer->lines[row + scroll_offset];
                char *new_line = buffer->lines[row + scroll_offset + 1];
                strcpy(new_line, current_line + col);
                current_line[col] = '\0';

                row++;
                col = 0;
                if (row >= LINES - 3) 
                {
                    scroll_offset++;
                    row--;
                }
                break;
            }
            case KEY_BACKSPACE:
            case 127:
            {
                if (col > 0) 
                {
                    col--;
                    delete_char(buffer->lines[row + scroll_offset], col, MAX_COLS);
                }
                break;
            }
            case KEY_DOWN:
            {
                if (row + scroll_offset < buffer->num_lines - 1) 
                {
                    if (row < LINES - 3) 
                    {
                        row++;
                    } 
                    else 
                    {
                        scroll_offset++;
                    }
                }
                break;
            }
            case KEY_UP:
            {
                if (row + scroll_offset > 0) 
                {
                    if (row > 0) 
                    {
                        row--;
                    } 
                    else 
                    {
                        scroll_offset--;
                    }
                }
                break;
            }
            case KEY_LEFT:
            {
                if (col > 0) 
                    col--;
                break;
            }
            case KEY_RIGHT:
            {
                if (col < MAX_COLS - 2) 
                    col++;
                break;
            }
            default:
            {
                if (ch >= 32 && ch <= 126) 
                {
                    insert_char(buffer->lines[row + scroll_offset], col, ch, MAX_COLS);
                    col++;
                }
                break;
            }
        }

        // afisam liniile din win1
        werase(win1);
        for (int i = 0; i < LINES - 2; i++) 
        {
            if (i + scroll_offset < buffer->num_lines) 
            {
                mvwprintw(win1, i + 1, 1, "%s", buffer->lines[i + scroll_offset]);
            }
        }
        box(win1, 0, 0);
        wmove(win1, row + 1, col + 1);
        wrefresh(win1);
    }

    // la apasarea F5 mergem la output in fereastra a doua
    handle_output_window(buffer, win2);
    // parse_lines(buffer->lines, buffer->num_lines);
}


