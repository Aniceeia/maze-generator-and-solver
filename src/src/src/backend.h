#ifndef BACKEND_H
#define BACKEND_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../inc/objects.h"

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define ALPHA 0.1
#define GAMMA 0.9
#define EPSILON 0.1
#define EPISODES 1000

#define DIRECTIONS 4

enum { STEP, STUCK, COLLISION };

int create_maze(Maze_t *maze);
void remove_maze(Maze_t *maze);
int load_maze(Maze_t *maze, char *file_name);
int create_cave(Cave_t *cave);
void remove_cave(Cave_t *cave);
int load_cave(Cave_t *cave, char *file_name);
void generate_maze(Maze_t *maze);
void follow_wall(Maze_t *maze, Path_t *path);
int can_move(Maze_t *maze, Path_t *path);
void step_forward(Path_t *path);
void turn_left(Path_t *path);
void turn_right(Path_t *path);
void generate_cave(Cave_t *cave, float chance);
void change_gen(Cave_t *cave_prev, Cave_t *cave_next, int life, int death);
int amount(Cave_t *cave, int im, int jm);
int edge(Cave_t *cave, int i, int j);
// Функция обновления позиции с учетом стен
int update_position(int *x, int *y, int action, Maze_t *maze);
void remove_matrix(double **q_table, int rows);
void train_agent(Maze_t *maze, double **q_table, int x_start, int y_start,
                 int x_goal, int y_goal);
double **create_q_table(int num_states);
int choose_action(int state, double **q_table, double epsilon);
void update_q_table(double **q_table, int state, int action, int reward,
                    int next_state);
void remove_matrix(double **q_table, int rows);
//Aisha's help functions
int  convert_to_mazet(matrix_t matrix, Maze_t *maze);
int del_right_wall(int *arr, matrix_t * matrix);
int uniq_el(int * arr, matrix_t matrix,  int row);
int conc(int * arr, int src, int rec, int cols);
int countel(int* str, matrix_t matrix, int element, int rows);
int s21_create_matrix(int rows, int columns, matrix_t *result);
int reading(char* name, matrix_t * matrix);
int print_m_file(matrix_t matrix, FILE * file);
int print_m(matrix_t matrix);
int eller(int rows, int cols, matrix_t* matrix, Maze_t* maze);
void remove_matrix_from_matrx_t(int **matrix, int rows);
#endif  // BACKEND_H