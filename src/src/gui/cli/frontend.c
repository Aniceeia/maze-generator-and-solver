#include "frontend.h"

void draw_maze(Maze_t* maze) {
  int color = 1;
  for (int y = 0; y < 2 * maze->size_y; y++) {
    attron(COLOR_PAIR(color));
    color = 1;
    if (y % 2 != 1) color = 2;
    mvprintw(y + 1, 0, "  ");
    attroff(COLOR_PAIR(color));
  }
  for (int x = 0; x < 2 * maze->size_x + 1; x++) {
    attron(COLOR_PAIR(color));
    color = 1;
    if (x % 2) color = 2;
    mvprintw(0, 2 * x, "  ");
    attroff(COLOR_PAIR(color));
  }
  for (int y = 0; y < maze->size_y; y++) {
    for (int x = 0; x < maze->size_x; x++) {
      if (maze->cells[y][x].filled) {
        attron(COLOR_PAIR(maze->cells[y][x].color));
        mvprintw(2 * y + 1, 4 * x + 2, "    ");
        attroff(COLOR_PAIR(maze->cells[y][x].color));
        attron(COLOR_PAIR(3));
        mvprintw(2 * y + 2, 4 * x + 2, "    ");
        attroff(COLOR_PAIR(3));
      }
      if (maze->cells[y][x].floor) {
        attron(COLOR_PAIR(1));
        mvprintw(2 * y + 2, 4 * x + 2, "  ");
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(2));
        mvprintw(2 * y + 2, 4 * x + 4, "  ");
        attroff(COLOR_PAIR(2));
      }
      if (maze->cells[y][x].wall) {
        attron(COLOR_PAIR(1));
        mvprintw(2 * y + 1, 4 * x + 4, "  ");
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(2));
        mvprintw(2 * y + 2, 4 * x + 4, "  ");
        attroff(COLOR_PAIR(2));
      }
    }
  }
}

void draw_maze_web(Maze_t* maze) {
  printf("FLOORS\n");
  for (int y = 0; y < maze->size_y; y++) {
    for (int x = 0; x < maze->size_x; x++) {
      if (maze->cells[y][x].floor) {
        printf("1 ");
      } else {
        printf("0 ");
      }
    }
    printf("\n");
  }
  printf("WALLS\n");
  for (int y = 0; y < maze->size_y; y++) {
    for (int x = 0; x < maze->size_x; x++) {
      if (maze->cells[y][x].wall) {
        printf("1 ");
      } else {
        printf("0 ");
      }
    }
    printf("\n");
  }
}

void draw_solution_web(Maze_t* maze) {
  printf("SOLUTION\n");
  for (int y = 0; y < maze->size_y; y++) {
    for (int x = 0; x < maze->size_x; x++) {
      if (maze->cells[y][x].filled) {
        printf("%d ", maze->cells[y][x].color - 2);
      } else {
        printf("0 ");
      }
    }
    printf("\n");
  }
}

void draw_cave(Cave_t* cave) {
  for (int y = 0; y < cave->size_y; y++) {
    for (int x = 0; x < cave->size_x; x++) {
      if (cave->cells[y][x]) {
        attron(COLOR_PAIR(1));
        mvprintw(2 * y + 2, 4 * x + 4, "  ");
        mvprintw(2 * y + 1, 4 * x + 2, "  ");
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(2));
        mvprintw(2 * y + 1, 4 * x + 4, "  ");
        mvprintw(2 * y + 2, 4 * x + 2, "  ");
        attroff(COLOR_PAIR(2));
      }
    }
  }
}

void draw_cave_web(Cave_t* cave) {
  printf("MATRIX_START\n");
  for (int y = 0; y < cave->size_y; y++) {
    for (int x = 0; x < cave->size_x; x++) {
      printf("%d ", cave->cells[y][x]);
    }
    printf("\n");
  }
}
