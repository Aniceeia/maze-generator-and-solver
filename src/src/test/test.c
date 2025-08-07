#include <check.h>

#include "../src/backend.h"

START_TEST(maze_init_1) {
  Maze_t maze;
  char *filename = "./test/test_maze.txt";
  int err = load_maze(&maze, filename);
  ck_assert_int_eq(err, 0);
  remove_maze(&maze);
}
END_TEST

START_TEST(maze_init_2) {
  Maze_t maze;
  maze.size_x = 10;
  maze.size_y = 10;
  create_maze(&maze);
  generate_maze(&maze);
  ck_assert_int_eq(maze.cells[0][0].filled, 0);
  remove_maze(&maze);
}
END_TEST

START_TEST(maze_init_3) {
  Maze_t maze;
  char *filename = "fff";
  int err = load_maze(&maze, filename);
  ck_assert_int_eq(err, 1);
  if (!err) remove_maze(&maze);
}
END_TEST

START_TEST(solve_1) {
  Maze_t maze;
  maze.size_x = 10;
  maze.size_y = 10;
  create_maze(&maze);
  generate_maze(&maze);
  int rows = maze.size_y, cols = maze.size_x;
  int num_states = rows * cols;
  double **q_table = create_q_table(num_states);
  train_agent(&maze, q_table, 0, 0, 5, 4);
  int x = 0, y = 0;
  while (!(x == 5 && y == 4)) {
    int state = x * rows + y;
    int action = choose_action(state, q_table, 0.0);
    if (update_position(&x, &y, action, &maze) == STUCK) break;
  }
  remove_matrix(q_table, num_states);
  ck_assert_int_eq(x, 5);
  remove_maze(&maze);
}
END_TEST

START_TEST(solve_2) {
  Maze_t maze;
  Path_t path = {0, 0, 'r'};
  maze.size_x = 10;
  maze.size_y = 10;
  create_maze(&maze);
  generate_maze(&maze);
  path.x = 0;
  path.y = 0;
  maze.cells[4][5].color = 3;
  while (!(path.x == 5 && path.y == 4)) follow_wall(&maze, &path);
  ck_assert_int_eq(path.x, 5);
  remove_maze(&maze);
}
END_TEST

START_TEST(cave_init_1) {
  Cave_t cave;
  char *filename = "./test/test_cave.txt";
  int err = load_cave(&cave, filename);
  ck_assert_int_eq(err, 0);
  remove_cave(&cave);
}
END_TEST

START_TEST(cave_init_2) {
  Cave_t cave;
  cave.size_x = 10;
  cave.size_y = 10;
  create_cave(&cave);
  generate_cave(&cave, 0.7);
  ck_assert_int_eq(cave.cells[0][0] * 0, 0);
  remove_cave(&cave);
}
END_TEST

START_TEST(cave_init_3) {
  Cave_t cave;
  char *filename = "fff";
  int err = load_cave(&cave, filename);
  ck_assert_int_eq(err, 1);
  if (!err) remove_cave(&cave);
}
END_TEST

START_TEST(cave_change) {
  Cave_t cave;
  cave.size_x = 10;
  cave.size_y = 10;
  Cave_t cave_next;
  cave_next.size_x = 10;
  cave_next.size_y = 10;
  create_cave(&cave);
  create_cave(&cave_next);
  generate_cave(&cave, 0.7);
  change_gen(&cave, &cave_next, 4, 4);
  ck_assert_int_eq(cave.cells[0][0] * 0, 0);
  remove_cave(&cave);
  remove_cave(&cave_next);
}
END_TEST

Suite *maze_suite() {
  Suite *s = suite_create("maze_suite");
  TCase *tc = tcase_create("maze_tc");

  tcase_add_test(tc, maze_init_1);
  tcase_add_test(tc, maze_init_2);
  tcase_add_test(tc, maze_init_3);
  tcase_add_test(tc, solve_1);
  tcase_add_test(tc, solve_2);
  tcase_add_test(tc, cave_init_1);
  tcase_add_test(tc, cave_init_2);
  tcase_add_test(tc, cave_init_3);
  tcase_add_test(tc, cave_change);
  suite_add_tcase(s, tc);

  return s;
}

int main() {
  Suite *s = maze_suite();
  SRunner *sr = srunner_create(s);
  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_VERBOSE);
  int tf = srunner_ntests_failed(sr);
  srunner_free(sr);

  return tf > 0;
}