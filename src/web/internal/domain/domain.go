package domain

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"os/exec"
	"strconv"
	"strings"
)

type Maze_t struct {
	Rows   int
	Cols   int
	Floors [][]int
	Walls  [][]int
}

type Cave_t struct {
	State [][]int
	Life  int
	Death int
	Rows  int
	Cols  int
}

type Solve_t struct {
	Method int
	StartX int
	StartY int
	EndX   int
	EndY   int
}

type Step_t struct {
	State [][]int
	Life  int
	Death int
}

func parseLine(line string) []int {
	var row []int
	for _, num := range strings.Fields(line) {
		n, _ := strconv.Atoi(num)
		row = append(row, n)
	}
	return row
}
func handleCommand(cmd *exec.Cmd, input string) (*bufio.Scanner, error) {
	stdin, err := cmd.StdinPipe()
	if err != nil {
		return nil, err
	}

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		return nil, err
	}

	if err := cmd.Start(); err != nil {
		return nil, err
	}

	fmt.Fprint(stdin, input)
	stdin.Close()

	return bufio.NewScanner(stdout), nil
}

func GenerateMaze(r *http.Request) Maze_t {
	rows, _ := strconv.Atoi(r.FormValue("rows"))
	cols, _ := strconv.Atoi(r.FormValue("cols"))

	var walls, floors [][]int

	cmd := exec.Command("./maze", "--web")
	scanner, _ := handleCommand(cmd, fmt.Sprintf("GENERATE 1 %d %d\n", rows, cols))

	parseState := 0
	for scanner.Scan() {
		line := scanner.Text()
		switch {
		case line == "FLOORS":
			parseState = 1
			floors = make([][]int, 0, rows)
		case line == "WALLS":
			parseState = 2
			walls = make([][]int, 0, rows)
		case line == "WEB_READY" && parseState == 2:
			break
		default:
			if parseState == 1 {
				floors = append(floors, parseLine(line))
			} else if parseState == 2 {
				walls = append(walls, parseLine(line))
			}
		}
	}

	cmd.Wait()
	return Maze_t{
		Rows:   rows,
		Cols:   cols,
		Floors: floors,
		Walls:  walls,
	}
}

func LoadMaze(r *http.Request) Maze_t {
	var floors, walls [][]int
	file, header, _ := r.FormFile("file")

	defer file.Close()

	log.Printf("Received file: %s, size: %d", header.Filename, header.Size)

	tempFile, _ := os.CreateTemp("", "maze_*.txt")

	defer os.Remove(tempFile.Name())
	defer tempFile.Close()

	io.Copy(tempFile, file)

	tempFile.Close()

	cmd := exec.Command("./maze", "--web")
	input := fmt.Sprintf("GENERATE 0 %s\n", tempFile.Name())
	scanner, _ := handleCommand(cmd, input)

	parseState := 0
	for scanner.Scan() {
		line := scanner.Text()
		switch {
		case line == "FLOORS":
			parseState = 1
		case line == "WALLS":
			parseState = 2
		case line == "WEB_READY" && parseState == 2:
			break
		default:
			if parseState == 1 {
				floors = append(floors, parseLine(line))
			} else if parseState == 2 {
				walls = append(walls, parseLine(line))
			}
		}
	}

	cmd.Wait()

	if len(floors) > 0 {
		rows := len(floors)
		cols := len(floors[0])

		return Maze_t{
			Rows:   rows,
			Cols:   cols,
			Floors: floors,
			Walls:  walls,
		}
	}
	return Maze_t{}
}

func SolveMaze(tmpFile string, solve Solve_t) [][][]int {
	cmd := exec.Command("./maze", "--web")
	input := fmt.Sprintf("GENERATE 0 %s\nSOLVE %d %d %d %d %d\n",
		tmpFile, solve.Method, solve.StartX, solve.StartY, solve.EndX, solve.EndY)

	scanner, _ := handleCommand(cmd, input)

	solution := make([][][]int, 0)
	currentSolution := make([][]int, 0)
	inSolution := false

	for scanner.Scan() {
		line := scanner.Text()
		switch {
		case strings.HasPrefix(line, "SOLUTION"):
			inSolution = true
			if len(currentSolution) > 0 {
				solution = append(solution, currentSolution)
				currentSolution = make([][]int, 0)
			}
		case line == "WEB_READY":
			inSolution = false
			if len(currentSolution) > 0 {
				solution = append(solution, currentSolution)
			}
			break
		default:
			if inSolution {
				currentSolution = append(currentSolution, parseLine(line))
			}
		}
	}

	cmd.Wait()

	return solution
}

func runCaveCommand(input string) ([][]int, error) {
	cmd := exec.Command("./maze", "--web", "cave")

	var stderr bytes.Buffer
	cmd.Stderr = &stderr

	scanner, err := handleCommand(cmd, input)
	if err != nil {
		return nil, err
	}

	var state [][]int
	parsing := false

	for scanner.Scan() {
		line := scanner.Text()
		switch {
		case strings.HasPrefix(line, "MATRIX_START"):
			parsing = true
			state = [][]int{}
		case line == "WEB_READY":
			parsing = false
			break
		case parsing:
			state = append(state, parseLine(line))
		}
	}

	if err := cmd.Wait(); err != nil {
		errMsg := stderr.String()
		log.Printf("Command failed: %v\nStderr: %s", err, errMsg)
		return nil, fmt.Errorf("%v: %s", err, errMsg)
	}

	return state, nil
}

func GenerateCave(r *http.Request) [][]int {
	var state [][]int
	rows, _ := strconv.Atoi(r.FormValue("rows"))
	cols, _ := strconv.Atoi(r.FormValue("cols"))
	chance, _ := strconv.ParseFloat(r.FormValue("chance"), 64)

	log.Printf("Generating cave: rows=%d, cols=%d, chance=%.2f", rows, cols, chance)
	input := fmt.Sprintf("GENERATE 1 %d %d %.2f\n", rows, cols, chance)

	state, _ = runCaveCommand(input)

	return state
}

func LoadCave(r *http.Request) [][]int {
	file, header, _ := r.FormFile("file")

	defer file.Close()

	log.Printf("Received file: %s, size: %d", header.Filename, header.Size)

	tempFile, _ := os.CreateTemp("", "cave_*.txt")
	defer os.Remove(tempFile.Name())
	defer tempFile.Close()

	io.Copy(tempFile, file)

	tempFile.Close()

	input := fmt.Sprintf("GENERATE 0 %s\n", tempFile.Name())
	log.Printf("Sending command to C: %s", input)

	state, _ := runCaveCommand(input)

	return state
}

func StepCave(tempFile string, step Step_t) [][]int {
	file, _ := os.Create(tempFile)
	fmt.Fprintf(file, "%d %d\n", len(step.State), len(step.State[0]))
	for _, row := range step.State {
		strRow := make([]string, len(row))
		for i, num := range row {
			strRow[i] = strconv.Itoa(num)
		}
		fmt.Fprintln(file, strings.Join(strRow, " "))
	}
	file.Close()

	input := fmt.Sprintf("GENERATE 0 %s\nSET_LIMITS %d %d\nSTEP\n",
		tempFile, step.Life, step.Death)

	state, _ := runCaveCommand(input)

	return state
}
