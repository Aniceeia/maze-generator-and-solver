#include "backend.h"

int create_maze(Maze_t *maze) {
  int err = 0;
  maze->cells = calloc(sizeof(Cell_t *), maze->size_y);
  if (!maze->cells) err = 1;
  for (int i = 0; !err && i < maze->size_y; i++) {
    maze->cells[i] = calloc(sizeof(Cell_t), maze->size_x);
    if (!maze->cells[i]) err = 1;
  }
  return 0;
}

int create_cave(Cave_t *cave) {
  int err = 0;
  cave->cells = calloc(sizeof(int *), cave->size_y);
  if (!cave->cells) err = 1;
  for (int i = 0; !err && i < cave->size_y; i++) {
    cave->cells[i] = calloc(sizeof(int), cave->size_x);
    if (!cave->cells[i]) err = 1;
  }
  return err;
}

void remove_maze(Maze_t *maze) {
  for (int i = 0; maze->cells && i < maze->size_y; i++) free(maze->cells[i]);
  free(maze->cells);
}

void remove_cave(Cave_t *cave) {
  for (int i = 0; cave->cells && i < cave->size_y; i++) free(cave->cells[i]);
  free(cave->cells);
}

int load_maze(Maze_t *maze, char *file_name) {
  int err = 0;
  FILE *file = fopen(file_name, "r");
  if (!file) {
    err = 1;
  } else {
    fscanf(file, "%d %d", &maze->size_y, &maze->size_x);
    err = create_maze(maze);
  }

  for (int y = 0; !err && y < maze->size_y; y++)
    for (int x = 0; x < maze->size_x; x++)
      fscanf(file, "%d", &maze->cells[y][x].wall);

  for (int y = 0; !err && y < maze->size_y; y++)
    for (int x = 0; x < maze->size_x; x++)
      fscanf(file, "%d", &maze->cells[y][x].floor);

  if (file) fclose(file);
  return err;
}

int load_cave(Cave_t *cave, char *file_name) {
  int err = 0;
  FILE *file = fopen(file_name, "r");
  if (!file) {
    err = 1;
  } else {
    fscanf(file, "%d %d", &cave->size_y, &cave->size_x);
    err = create_cave(cave);
    for (int y = 0; !err && y < cave->size_y; y++)
      for (int x = 0; x < cave->size_x; x++)
        fscanf(file, "%d", &cave->cells[y][x]);
  }

  if (file) fclose(file);
  return err;
}

// maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze

void generate_maze(Maze_t *maze) {
  matrix_t mat = {0};
  int size1 = maze->size_x;
  int size2 = maze->size_y;
  s21_create_matrix(size1, size2, &mat);
  eller(size1, size2, &mat, maze);
  remove_matrix_from_matrx_t(mat.matrix1, size1);
  remove_matrix_from_matrx_t(mat.matrix_, size1);
}

void turn_right(Path_t *path) {
  switch (path->dir) {
    case 'u':
      path->dir = 'r';
      break;
    case 'l':
      path->dir = 'u';
      break;
    case 'd':
      path->dir = 'l';
      break;
    default:
      path->dir = 'd';
  }
}

void turn_left(Path_t *path) {
  switch (path->dir) {
    case 'u':
      path->dir = 'l';
      break;
    case 'l':
      path->dir = 'd';
      break;
    case 'd':
      path->dir = 'r';
      break;
    default:
      path->dir = 'u';
  }
}

void step_forward(Path_t *path) {  // здесь происходит отрисовка пути
  switch (path->dir) {
    case 'u':
      if (path->y != 0) path->y--;
      break;
    case 'd':
      path->y++;
      break;
    case 'l':
      if (path->x != 0) path->x--;
      break;
    default:
      path->x++;
  }
}

int can_move(Maze_t *maze, Path_t *path) {
  int result = 0;
  switch (path->dir) {
    case 'u':
      result = (path->y != 0 && maze->cells[path->y - 1][path->x].floor == 0);
      break;
    case 'd':
      result = (maze->cells[path->y][path->x].floor == 0);
      break;
    case 'l':
      result = (path->x != 0 && maze->cells[path->y][path->x - 1].wall == 0);
      break;
    default:
      result = (maze->cells[path->y][path->x].wall == 0);
  }
  return result;
}

void follow_wall(Maze_t *maze, Path_t *path) {
  turn_right(path);
  if (can_move(maze, path)) {
    step_forward(path);
  } else {
    turn_left(path);
    if (can_move(maze, path)) {
      step_forward(path);
    } else {
      turn_left(path);
      if (can_move(maze, path)) {
        step_forward(path);
      } else {
        turn_left(path);
      }
    }
  }
}

// Функция обновления позиции с учетом стен
int update_position(int *x, int *y, int action, Maze_t *maze) {
  int new_x = *x;
  int new_y = *y;
  switch (action) {
    case UP:
      if (new_y != 0 && maze->cells[*y - 1][*x].floor == 0) new_y--;
      break;
    case RIGHT:
      if (new_x != maze->size_x - 1 && maze->cells[*y][*x].wall == 0) new_x++;
      break;
    case DOWN:
      if (new_y != maze->size_y - 1 && maze->cells[*y][*x].floor == 0) new_y++;
      break;
    case LEFT:
      if (new_x != 0 && maze->cells[*y][*x - 1].wall == 0) new_x--;
      break;
  }
  if (new_x == *x && new_y == *y) {
    return COLLISION;
  }
  *x = new_x;
  *y = new_y;
  return STEP;
}

double **create_q_table(int num_states) {
  double **q_table = calloc(num_states, sizeof(double *));
  for (int i = 0; i < num_states; i++) {
    q_table[i] = calloc(DIRECTIONS, sizeof(double));
  }
  return q_table;
}

// Выбор действия с использованием эпсилон-жадной стратегии
int choose_action(int state, double **q_table, double epsilon) {
  if ((double)rand() / RAND_MAX < epsilon) {
    return rand() % DIRECTIONS;
  } else {
    int best_action = 0;
    for (int i = 1; i < DIRECTIONS; i++) {
      if (q_table[state][i] > q_table[state][best_action]) {
        best_action = i;
      }
    }
    return best_action;
  }
}

// Обновление Q-таблицы
void update_q_table(double **q_table, int state, int action, int reward,
                    int next_state) {
  double max_q_next = q_table[next_state][0];
  for (int i = 1; i < DIRECTIONS; i++) {
    if (q_table[next_state][i] > max_q_next) {
      max_q_next = q_table[next_state][i];
    }
  }
  q_table[state][action] +=
      ALPHA * (reward + GAMMA * max_q_next - q_table[state][action]);
}

void train_agent(Maze_t *maze, double **q_table, int x_start, int y_start,
                 int x_goal, int y_goal) {
  int cols = maze->size_x;
  double epsilon = EPSILON;
  for (int ep = 0; ep < EPISODES; ep++) {
    int x = x_start, y = y_start;
    int steps = 0;
    epsilon = 0.9 * (1.0 - (float)ep / EPISODES);  // Динамическое уменьшение ε
    while (!(x == x_goal && y == y_goal) && steps < EPISODES * 10) {
      int state = x * cols + y;
      int action = choose_action(state, q_table, epsilon);
      int reward = -1;
      int result = update_position(&x, &y, action, maze);
      if (result == COLLISION) {
        reward = -5;
      } else if (x == x_goal && y == y_goal) {
        reward = 100;
      }
      int next_state = x * cols + y;
      update_q_table(q_table, state, action, reward, next_state);
      steps++;
    }
    if (steps == EPISODES * 10) {
      // printf("Episode %d exceeded step limit. Exiting.\n", ep + 1);
    }
  }
}

void remove_matrix(double **q_table, int rows) {
  for (int i = 0; i < rows; i++) {
    free(q_table[i]);
  }
  free(q_table);
}

// maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze-maze

// cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave

void generate_cave(Cave_t *cave, float chance) {
  for (int i = 0; i < cave->size_y; i++)
    for (int j = 0; j < cave->size_x; j++)
      cave->cells[i][j] = ((rand() % 10) / 10.0) >= (1.0 - chance);
}

void change_gen(Cave_t *cave_prev, Cave_t *cave_next, int life, int death) {
  for (int i = 0; i < cave_prev->size_y; i++) {
    for (int j = 0; j < cave_prev->size_x; j++) {
      int neighbors = amount(cave_prev, i, j);
      cave_next->cells[i][j] = cave_prev->cells[i][j]
                                   ? (neighbors >= life) && (neighbors <= death)
                                   : (neighbors == life);
    }
  }
  for (int i = 0; i < cave_prev->size_y; i++)
    for (int j = 0; j < cave_prev->size_x; j++)
      cave_prev->cells[i][j] = cave_next->cells[i][j];
}

int amount(Cave_t *cave, int im, int jm) {
  int sum = 0;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      if (i == 0 && j == 0) continue;     // Skip the cell itself
      sum += edge(cave, im + i, jm + j);  // Count neighbors
    }
  }
  return sum;
}

int edge(Cave_t *cave, int i, int j) {
  // Wrap around edges
  if (i < 0) i = cave->size_y - 1;
  if (i >= cave->size_y) i = 0;
  if (j < 0) j = cave->size_x - 1;
  if (j >= cave->size_x) j = 0;
  return cave->cells[i][j];
}

// eller algo Aisha's backend
int convert_to_mazet(matrix_t matrix, Maze_t *maze) {
  int err = 0;
  maze->size_x = matrix.columns;
  maze->size_y = matrix.rows;
  create_maze(maze);
  for (int i = 0; i < matrix.rows; i++) {
    for (int j = 0; j < matrix.columns; j++) {
      maze->cells[i][j].wall = matrix.matrix1[i][j];
      maze->cells[i][j].floor = matrix.matrix_[i][j];
      maze->cells[i][j].filled = 0;
    }
  }
  return err;
}

int del_right_wall(int *arr, matrix_t *matrix) {
  for (int i = 0; i < matrix->columns - 1; i++) {
    matrix->matrix_[matrix->rows - 1][i] = 1;
    if (arr[i] != arr[i + 1]) {
      matrix->matrix1[matrix->rows - 1][i] = 0;
      // arr[i+1]=arr[i];
      conc(arr, arr[i], arr[i + 1], matrix->columns);
      // arr[i]=arr[i+1];
    }
  }
  return 0;
}

int uniq_el(int *arr, matrix_t matrix, int row) {
  for (int i = 0; i < matrix.columns; i++) {
    if (matrix.matrix_[row][i] == 1) {
      arr[i] = matrix.columns * (row + 1) + i;
    }
    // matrix.columns*row
  }
  return 0;
}

int conc(int *arr, int src, int rec, int cols) {
  if (src > rec) {
    int temp = src;
    src = rec;
    rec = temp;
  }
  for (int i = 0; i < cols; i++) {
    if (arr[i] == rec) {
      arr[i] = src;
    }
  }
  return 0;
}

int countel(int *str, matrix_t matrix, int element, int rows) {
  int count = 0;
  for (int i = 0; i < matrix.columns; i++) {
    if (str[i] == element && matrix.matrix_[rows][i] == 0) {
      count++;
    }
  }

  return count;
}

// add checks
int s21_create_matrix(int rows, int columns, matrix_t *result) {
  if (result == NULL || rows < 1 || columns < 1) {
    return 1;
  }
  result->rows = rows;
  result->columns = columns;
  result->matrix_ = (int **)malloc(rows * sizeof(int *));
  result->matrix1 = (int **)malloc(rows * sizeof(int *));
  for (int i = 0; i < rows; i++) {
    result->matrix_[i] = (int *)malloc(columns * sizeof(int));
    result->matrix1[i] = (int *)malloc(columns * sizeof(int));
    for (int j = 0; j < columns; j++) {
      result->matrix_[i][j] = 0;
      result->matrix1[i][j] = 0;
    }
  }

  return 0;
}

int reading(char *name, matrix_t *matrix) {
  // matrix =
  // s21_create_matrix()
  FILE *f = fopen(name, "r");
  int rows, cols;
  fscanf(f, "%d %d", &rows, &cols);
  s21_create_matrix(rows, cols, matrix);
  for (int i = 0; i < matrix->rows; i++) {
    for (int j = 0; j < matrix->columns; j++) {
      fscanf(f, "%d ", &matrix->matrix1[i][j]);
    }
  }
  for (int i = 0; i < matrix->rows; i++) {
    for (int j = 0; j < matrix->columns; j++) {
      fscanf(f, "%d ", &matrix->matrix_[i][j]);
    }
  }
  return 0;
}

int print_m_file(matrix_t matrix, FILE *file) {
  // FILE * file = fopen("file_maze.txt", "w");

  for (int i = 0; i < matrix.columns; i++) {
    fprintf(file, "%s", " _");
  }
  fprintf(file, "%s", " ");
  fprintf(file, "%s", "\n");
  for (int i = 0; i < matrix.rows; i++) {
    fprintf(file, "%c", '|');
    for (int j = 0; j < matrix.columns; j++) {
      if (matrix.matrix_[i][j] == 0) {
        fprintf(file, "%c", ' ');
      } else {
        fprintf(file, "%s", "_");
      }
      if (matrix.matrix1[i][j] == 0) {
        fprintf(file, "%s", " ");
      } else {
        fprintf(file, "%c", '|');
      }
    }
    fprintf(file, "%s", "\n");
  }
  fclose(file);
  return 0;
}

// n:cols m:rows
int print_m(matrix_t matrix) {
  for (int i = 0; i < matrix.columns; i++) {
    printf("%s", " _");
  }
  printf("%s", " ");
  printf("%s", "\n");
  for (int i = 0; i < matrix.rows; i++) {
    printf("%c", '|');
    for (int j = 0; j < matrix.columns; j++) {
      if (matrix.matrix_[i][j] == 0) {
        printf("%c", ' ');
      } else {
        printf("%s", "_");
      }
      if (matrix.matrix1[i][j] == 0) {
        printf("%s", " ");
      } else {
        printf("%c", '|');
      }
    }
    printf("%s", "\n");
  }

  return 0;
}

int eller(int rows, int cols, matrix_t *matrix, Maze_t *maze) {
  srand(time(NULL));

  int *f_s = (int *)malloc(cols * sizeof(int));
  for (int i = 0; i < cols; i++) {
    f_s[i] = i;
  }
  // int * s_s = (int *)malloc(cols * sizeof(int));

  // тут правые стены
  for (int r = 0; r < rows; r++) {
    // расставляем правые стенки
    for (int c = 0; c < cols - 1; c++) {
      // f_s[c]=c;
      if ((f_s[c] == f_s[c + 1]) || rand() % 10 > 6) {
        matrix->matrix1[r][c] = 1;
      } else {
        conc(f_s, f_s[c], f_s[c + 1], cols);
      }
    }
    matrix->matrix1[r][cols - 1] = 1;
    // расставляем нижние стены
    for (int c = 0; c < cols; c++) {
      // f_s[c]=c;
      if (rand() % 10 > 6) {
        matrix->matrix_[r][c] = 0;
      } else if (countel(f_s, *matrix, f_s[c], r) <= 1) {
        matrix->matrix_[r][c] = 0;
      } else {
        matrix->matrix_[r][c] = 1;
      }
    }

    for (int c = 0; c < cols; c++) {
      // f_s[c]=c;
      if (countel(f_s, *matrix, f_s[c], r) <= 1) {
        matrix->matrix_[r][c] = 0;
      }
    }
    // Удалить ячейки с нижней стенкой из их набора->
    // Присоедините все ячейки, не входящие в набор,
    //  к их собственному уникальному набору.
    if (r != rows - 1) {  //?
      uniq_el(f_s, *matrix, r);
    }
  }
  matrix->matrix_[matrix->rows - 1][matrix->columns - 1] = 1;
  // удаляем лишние правые стены на дне
  del_right_wall(f_s, matrix);
  convert_to_mazet(*matrix, maze);
  free(f_s);
  return 0;
}

void remove_matrix_from_matrx_t(int **matrix, int rows) {
  for (int i = 0; i < rows; i++) {
    free(matrix[i]);
  }
  free(matrix);
}
// cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave-cave
