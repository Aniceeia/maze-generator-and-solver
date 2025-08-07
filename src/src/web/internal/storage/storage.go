package storage

import (
	"fmt"
	"os"
	"strconv"
	"strings"
)

type Maze_t struct {
	Rows   int
	Cols   int
	Floors [][]int
	Walls  [][]int
}

func SaveMazeToFile(filename string, maze Maze_t) error {
	if len(maze.Floors) == 0 || len(maze.Walls) == 0 {
		return fmt.Errorf("maze not generated")
	}

	file, err := os.Create(filename)
	if err != nil {
		return err
	}
	defer file.Close()

	_, err = fmt.Fprintf(file, "%d %d\n", maze.Rows, maze.Cols)
	if err != nil {
		return err
	}

	for _, row := range maze.Walls {
		strRow := make([]string, len(row))
		for i, num := range row {
			strRow[i] = strconv.Itoa(num)
		}
		_, err = fmt.Fprintln(file, strings.Join(strRow, " "))
		if err != nil {
			return err
		}
	}

	_, err = fmt.Fprintln(file)
	if err != nil {
		return err
	}

	for _, row := range maze.Floors {
		strRow := make([]string, len(row))
		for i, num := range row {
			strRow[i] = strconv.Itoa(num)
		}
		_, err = fmt.Fprintln(file, strings.Join(strRow, " "))
		if err != nil {
			return err
		}
	}

	return nil
}
