#include <string.h>

#include "gui/cli/frontend.h"
#include "src/backend.h"

void solve_maze(int web, Maze_t *maze, int x_start, int y_start, int x_goal,
                int y_goal) {
  Path_t path = {0, 0, 'r'};
  int ch = 0;
  path.x = x_start;
  path.y = y_start;
  maze->cells[path.y][path.x].filled = 1;
  maze->cells[path.y][path.x].color = 4;
  maze->cells[y_goal][x_goal].color = 3;
  while (!(path.x == x_goal && path.y == y_goal)) {
    maze->cells[path.y][path.x].color = 3;
    timeout(50);
    ch = getch();
    if (ch == 'q') break;
    follow_wall(maze, &path);
    maze->cells[path.y][path.x].filled = 1;
    maze->cells[path.y][path.x].color = 4;
    if (!web) {
      draw_maze(maze);
    }
    if (web) {
      // solve_maze отрисовывваем поэтапно от матрицы к матрице, где 2 - то кула
      // мы зашли, а 1 - путь, здесь ищем не кратчайший путь
      draw_solution_web(maze);
    }
  }
  timeout(-1);
}

void solve_ml(int web, Maze_t *maze, int x_start, int y_start, int x_goal,
              int y_goal) {
  int rows = maze->size_y, cols = maze->size_x;
  int num_states = rows * cols;
  double **q_table = create_q_table(num_states);
  train_agent(maze, q_table, x_start, y_start, x_goal, y_goal);
  int x = x_start, y = y_start;
  maze->cells[y][x].color = 3;
  maze->cells[y][x].filled = 1;
  while (!(x == x_goal && y == y_goal)) {
    timeout(50);
    int state = x * rows + y;
    int action = choose_action(state, q_table, 0.0);
    if (update_position(&x, &y, action, maze) == STUCK) break;
    maze->cells[y][x].color = 3;
    maze->cells[y][x].filled = 1;
    if (!web) {
      draw_maze(maze);
    }
  }
  // ml - решение отрисовывает только конечное решение, тк мл ищет только его
  draw_solution_web(maze);
  timeout(-1);
  remove_matrix(q_table, num_states);
}

void reset_solution(Maze_t *maze) {
  for (int y = 0; y < maze->size_y; y++) {
    for (int x = 0; x < maze->size_x; x++) {
      if (maze->cells[y][x].color == 3 || maze->cells[y][x].color == 4) {
        maze->cells[y][x].filled = 0;
        maze->cells[y][x].color = 0;
      }
    }
  }
}

void web_interactive_loop(Maze_t *maze, int x_start, int y_start, int x_goal,
                          int y_goal) {
  char command[100];
  int current_x_start = x_start;
  int current_y_start = y_start;
  int current_x_goal = x_goal;
  int current_y_goal = y_goal;
  while (1) {
    printf("WEB_READY\n");
    fflush(stdout);

    if (fgets(command, sizeof(command), stdin) == NULL) break;
    command[strcspn(command, "\n")] = '\0';

    if (strncmp(command, "SOLVE", 5) == 0) {
      int method, new_x_start, new_y_start, new_x_goal, new_y_goal;
      int args_parsed =
          sscanf(command + 6, "%d %d %d %d %d", &method, &new_x_start,
                 &new_y_start, &new_x_goal, &new_y_goal);

      if (args_parsed != 5) {
        printf(
            "WEB_ERROR Неверный формат команды SOLVE. Используйте: SOLVE "
            "метод x_start y_start x_goal y_goal\n");
        continue;
      }

      if (new_x_start < 0 || new_x_start >= maze->size_x || new_y_start < 0 ||
          new_y_start >= maze->size_y || new_x_goal < 0 ||
          new_x_goal >= maze->size_x || new_y_goal < 0 ||
          new_y_goal >= maze->size_y) {
        printf(
            "WEB_ERROR Координаты выходят за пределы лабиринта (0 <= x < "
            "%d, 0 <= y < %d)\n",
            maze->size_x, maze->size_y);
        continue;
      }

      current_x_start = new_x_start;
      current_y_start = new_y_start;
      current_x_goal = new_x_goal;
      current_y_goal = new_y_goal;

      reset_solution(maze);
      if (method == 0) {
        solve_maze(1, maze, current_x_start, current_y_start, current_x_goal,
                   current_y_goal);
      } else {
        solve_ml(1, maze, current_x_start, current_y_start, current_x_goal,
                 current_y_goal);
      }

    } else if (strncmp(command, "GENERATE", 8) == 0) {
      int gen_mode;
      int new_size_x, new_size_y;
      char filename[100];
      int args_parsed;

      if (sscanf(command + 9, "%d", &gen_mode) < 1) {
        printf("WEB_ERROR Неверный формат команды\n");
        continue;
      }

      if (gen_mode == 1) {
        args_parsed = sscanf(command + 9, "%d %d %d", &gen_mode, &new_size_x,
                             &new_size_y);
        if (args_parsed != 3 || new_size_x < 1 || new_size_x > 50 ||
            new_size_y < 1 || new_size_y > 50) {
          printf("WEB_ERROR Неверные размеры лабиринта (допустимо 1-50)\n");
          continue;
        }

        remove_maze(maze);

        maze->size_x = new_size_x;
        maze->size_y = new_size_y;

        if (create_maze(maze) != 0) {
          printf("WEB_ERROR Ошибка создания лабиринта\n");
          continue;
        }
        generate_maze(maze);
        draw_maze_web(maze);

      } else if (gen_mode == 0) {
        args_parsed = sscanf(command + 9, "%d %99s", &gen_mode, filename);
        if (args_parsed < 2) {
          printf("WEB_ERROR Укажите имя файла\n");
          continue;
        }

        remove_maze(maze);
        int err = load_maze(maze, filename);
        if (err) {
          printf("WEB_ERROR Ошибка загрузки файла\n");
        } else {
          draw_maze_web(maze);
        }
      } else {
        printf("WEB_ERROR Неверный режим GENERATE (допустимо 0 или 1)\n");
      }

    } else if (strcmp(command, "EXIT") == 0) {
      break;
    }
  }
}

void start_maze(int web) {
  Maze_t maze;
  char filename[100];
  int x_goal = 0, y_goal = 0, x_start = 0, y_start = 0, err = 0, load = 0,
      solve = 0, method = 0;

  if (web) {
    create_maze(&maze);
    web_interactive_loop(&maze, x_start, y_start, x_goal, y_goal);
  }
  if (!web) {
    if (!scanf("%d", &load)) load = -1;
    if (load == 1) {
      scanf("%s", filename);
      err = load_maze(&maze, filename);
    } else if (load == 0) {
      if (scanf("%d %d", &maze.size_y, &maze.size_x) != 2 || maze.size_x < 1 ||
          maze.size_y < 1 || maze.size_x > 50 || maze.size_y > 50)
        err = 1;
      if (!err) create_maze(&maze);
    } else {
      err = 1;
    }
    if (!err) {
      if (!scanf("%d", &solve) || !(solve == 0 || solve == 1)) err = 1;
      if (!err && solve &&
          (!scanf("%d", &method) || !(method == 0 || method == 1)))
        err = 1;
      if (!err && solve) {
        if (!err && (scanf("%d %d", &x_start, &y_start) != 2 || x_start < 0 ||
                     x_start > maze.size_x - 1 || y_start < 0 ||
                     y_start > maze.size_y - 1))
          err = 1;
        if (!err)
          if (scanf("%d %d", &x_goal, &y_goal) != 2 || x_goal < 0 ||
              x_goal > maze.size_x - 1 || y_goal < 0 ||
              y_goal > maze.size_y - 1)
            err = 1;
      }
      WIN_INIT(-1);
      int ch = 0;
      while (!err && ch != 'q') {
        if (!load && ch != 'd') generate_maze(&maze);
        draw_maze(&maze);
        if (ch == 'd' && solve && !method)
          solve_maze(web, &maze, x_start, y_start, x_goal, y_goal);
        if (ch == 'd' && solve && method)
          solve_ml(web, &maze, x_start, y_start, x_goal, y_goal);
        ch = getch();
        clear();
      }
      endwin();
    }

    if (!err) remove_maze(&maze);
  }
}

void start_cave(int web) {
  Cave_t cave, cave_next;
  int err = 0, load = 0, life = 0, death = 0, update = 0;
  float chance = 0;
  char filename[100];
  if (!scanf("%d", &load)) load = -1;
  if (load == 1) {
    scanf("%s", filename);
    err = load_cave(&cave, filename);
    if (!err) {
      cave_next.size_x = cave.size_x;
      cave_next.size_y = cave.size_y;
      create_cave(&cave_next);
    }
  } else if (load == 0) {
    if (scanf("%d %d", &cave.size_y, &cave.size_x) != 2 || cave.size_x < 1 ||
        cave.size_y < 1 || cave.size_x > 50 || cave.size_y > 50)
      err = 1;
    if (!err && !scanf("%f", &chance)) err = 1;
    if (!err) {
      cave_next.size_x = cave.size_x;
      cave_next.size_y = cave.size_y;
      create_cave(&cave);
      create_cave(&cave_next);
      generate_cave(&cave, chance);
    }
  } else {
    err = 1;
  }
  if (!err) {
    if (scanf("%d %d", &life, &death) != 2 || life < 1 || death < 1 ||
        life > 7 || death > 7)
      err = 1;
    if (!err && !scanf("%d", &update)) err = 1;
    WIN_INIT(update == 0 ? -1 : update);
    int ch = 0;
    while (!err && ch != 'q') {
      if (!web) {
        draw_cave(&cave);
      } else {
        draw_cave_web(&cave);
      }
      ch = getch();
      change_gen(&cave, &cave_next, life, death);
      clear();
    }
    if (!err) {
      remove_cave(&cave);
      remove_cave(&cave_next);
    }
    endwin();
  }
}

void web_interactive_loop_cave() {
  char command[100];
  Cave_t cave = {0}, cave_next = {0};
  int cave_initialized = 0;
  int current_life = 0, current_death = 0;

  while (1) {
    printf("WEB_READY\n");
    fflush(stdout);

    if (fgets(command, sizeof(command), stdin) == NULL) break;
    command[strcspn(command, "\n")] = '\0';

    if (strncmp(command, "GENERATE", 8) == 0) {
      int gen_mode;
      int args_parsed;
      int size_x, size_y;
      float chance;
      char filename[100];

      // Освобождаем старые данные, если есть
      if (cave_initialized) {
        remove_cave(&cave);
        remove_cave(&cave_next);
        cave_initialized = 0;
      }

      // Парсим команду
      if (sscanf(command + 9, "%d", &gen_mode) < 1) {
        printf("WEB_ERROR Неверный формат команды GENERATE\n");
        continue;
      }

      if (gen_mode == 1) {  // Случайная генерация
        args_parsed = sscanf(command + 9, "%d %d %d %f", &gen_mode, &size_y,
                             &size_x, &chance);
        if (args_parsed != 4 || size_x < 1 || size_x > 50 || size_y < 1 ||
            size_y > 50 || chance < 0 || chance > 1) {
          printf(
              "WEB_ERROR Неверные параметры. Формат: GENERATE 1 size_y size_x "
              "chance (1-50, 0<=chance<=1)\n");
          continue;
        }

        // Инициализация структур
        cave.size_x = size_x;
        cave.size_y = size_y;
        cave_next.size_x = size_x;
        cave_next.size_y = size_y;

        if (create_cave(&cave) || create_cave(&cave_next)) {
          printf("WEB_ERROR Ошибка создания пещеры\n");
          continue;
        }

        generate_cave(&cave, chance);
        cave_initialized = 1;
        draw_cave_web(&cave);

      } else if (gen_mode == 0) {  // Загрузка из файла
        args_parsed = sscanf(command + 9, "%d %99s", &gen_mode, filename);
        if (args_parsed < 2) {
          printf("WEB_ERROR Укажите имя файла\n");
          continue;
        }

        // Загружаем пещеру из файла
        if (load_cave(&cave, filename)) {
          printf("WEB_ERROR Ошибка загрузки файла\n");
          continue;
        }

        // Создаем буфер для следующего поколения
        cave_next.size_x = cave.size_x;
        cave_next.size_y = cave.size_y;
        if (create_cave(&cave_next)) {
          remove_cave(&cave);
          printf("WEB_ERROR Ошибка создания буфера\n");
          continue;
        }

        // Копируем состояние в буфер
        for (int y = 0; y < cave.size_y; y++) {
          for (int x = 0; x < cave.size_x; x++) {
            cave_next.cells[y][x] = cave.cells[y][x];
          }
        }

        cave_initialized = 1;
        printf("MATRIX_START\n");
        for (int y = 0; y < cave.size_y; y++) {
          for (int x = 0; x < cave.size_x; x++) {
            printf("%d ", cave.cells[y][x]);
          }
          printf("\n");
        }
      } else {
        printf("WEB_ERROR Неверный режим GENERATE (0 или 1)\n");
      }

    } else if (strncmp(command, "SET_LIMITS", 10) == 0) {
      int life, death;
      if (sscanf(command + 11, "%d %d", &life, &death) != 2) {
        printf("WEB_ERROR Формат: SET_LIMITS life death\n");
        continue;
      }
      if (life < 1 || life > 7 || death < 1 || death > 7) {
        printf("WEB_ERROR Пределы должны быть 1-7\n");
        continue;
      }
      current_life = life;
      current_death = death;

    } else if (strncmp(command, "STEP", 4) == 0) {
      if (!cave_initialized) {
        printf("WEB_ERROR Сначала выполните GENERATE\n");
        continue;
      }
      change_gen(&cave, &cave_next, current_life, current_death);
      draw_cave_web(&cave);

    } else if (strcmp(command, "EXIT") == 0) {
      break;
    }
  }

  // Освобождение ресурсов
  if (cave_initialized) {
    remove_cave(&cave);
    remove_cave(&cave_next);
  }
}

int main(int argc, char *argv[]) {
  srand(time(0));
  srand(time(0));
  if (argc > 1 && (strcmp(argv[1], "--web") == 0)) {
    if (argc > 2 && strcmp(argv[2], "cave") == 0) {
      web_interactive_loop_cave();  // Веб-режим для пещеры
    } else {
      start_maze(1);  // Веб-режим для лабиринта
    }
  } else {
    int err = 0, type;
    if (!scanf("%d", &type) || !(type == 0 || type == 1)) err = 1;
    if (!err) (type == 0) ? start_maze(0) : start_cave(0);
  }

  return 0;
}
