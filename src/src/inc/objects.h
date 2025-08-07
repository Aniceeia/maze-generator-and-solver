#ifndef OBJECTS_H
#define OBJECTS_H

typedef struct {
  int wall;
  int floor;
  int filled;
  int color;
} Cell_t;

typedef struct {
  int size_y;
  int size_x;
  Cell_t **cells;
} Maze_t;

typedef struct {
  int size_y;
  int size_x;
  int **cells;
} Cave_t;

typedef struct {
  int x;
  int y;
  char dir;
} Path_t;

typedef struct matrix_struct {
  int **matrix1;
  int **matrix_;
  int rows;
  int columns;
} matrix_t;

#endif  // OBJECTS_H