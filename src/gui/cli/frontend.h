#ifndef FRONTEND_H
#define FRONTEND_H

#include <ncurses.h>

#include "../../inc/objects.h"

#define WIN_INIT(time)                      \
  {                                         \
    initscr();                              \
    start_color();                          \
    init_pair(1, COLOR_BLACK, COLOR_WHITE); \
    init_pair(2, COLOR_BLACK, COLOR_GREEN); \
    init_pair(3, COLOR_BLACK, COLOR_RED);   \
    init_pair(4, COLOR_BLACK, COLOR_CYAN);  \
    noecho();                               \
    keypad(stdscr, TRUE);                   \
    curs_set(0);                            \
    timeout(time);                          \
  }

void draw_maze(Maze_t* maze);
void draw_maze_web(Maze_t* maze);
void draw_cave(Cave_t* cave);
void draw_cave_web(Cave_t* cave);
void draw_solution_web(Maze_t* maze);

#endif  // FRONTEND_H