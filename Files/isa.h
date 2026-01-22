#ifndef ISA_H
#define ISA_H

#include "parse_code.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_COLS 256

typedef struct {
    char **lines;
    int num_lines;
} Buffer;

// functii pentru interfata
void start_ncurses();
void end_ncurses(WINDOW *menuwin_left, WINDOW *menuwin_right);

// functii pentru buffer
Buffer *create_buffer();
void insert_char(char *line, int pos, char ch, int max_cols);
void delete_char(char *line, int pos, int max_cols);
void insert_new_line(Buffer *buffer, int line, int max_cols);
void free_buffer(Buffer *buffer);

// functii pentru diferite lucruri
void display_message(WINDOW *win, const char *format, ...);
void handle_error(WINDOW *win, const char *format, ...);

// functii pentru ferestre
void init_windows(WINDOW **win1, WINDOW **win2, int height, int width);
void handle_output_window(Buffer *buffer, WINDOW *win2);
void handle_input(Buffer *buffer, WINDOW *win1, WINDOW *win2);

#endif // ISA_H