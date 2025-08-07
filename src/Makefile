CFLAGS= -Wall -Werror
OS := $(shell uname -s)

ifeq ($(OS),Linux)
	OPEN_CMD = xdg-open
	TEST_LIBS = -lcheck -lsubunit -lm -pthread
endif
ifeq ($(OS),Darwin)
	OPEN_CMD = open
	TEST_LIBS = -lcheck
endif


all: install

install:
	gcc $(CFLAGS) ./src/backend.c ./gui/cli/frontend.c ./maze.c -g -o maze -lncurses

tests:
	gcc $(CFLAGS) -g -o test.out ./src/backend.c ./test/test.c $(TEST_LIBS)
	./test.out

gcov_report:
	gcc $(CFLAGS) --coverage ./src/backend.c ./test/test.c -o gcov_test $(TEST_LIBS)
	chmod +x gcov_test
	./gcov_test
	lcov -t "gcov_test" -o gcov_test.info --no-external -c -d .
	genhtml -o report/ gcov_test.info
	$(OPEN_CMD) ./report/index.html
	rm -rf gcov*

clang:
	clang-format -i -style=google *.c */*.c */*/*.c

rebuild: clean install

valgrind:
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all -s ./test.out

uninstall:
	rm -rf maze

clean:
	rm -rf maze test.out report
