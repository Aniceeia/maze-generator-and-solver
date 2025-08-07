package handlers

import (
	"encoding/json"
	"log"
	"net/http"
	"os"
	"strings"

	"github.com/google/uuid"

	"web/internal/domain"
	"web/internal/storage"
)

type mazeResponse struct {
	Floors    [][]int   `json:"floors,omitempty"`
	Walls     [][]int   `json:"walls,omitempty"`
	SolveAlgo [][][]int `json:"solveAlgo,omitempty"`
	SolveML   [][]int   `json:"solveML,omitempty"`
	Error     string    `json:"error,omitempty"`
}

type caveResponse struct {
	State [][]int `json:"state,omitempty"`
	Error string  `json:"error,omitempty"`
}

type stepRequest struct {
	State [][]int `json:"state"`
	Life  int     `json:"life"`
	Death int     `json:"death"`
}

type requestSolution struct {
	Maze   storage.Maze_t `json:"maze"`
	Method int            `json:"method"`
	StartX int            `json:"startX"`
	StartY int            `json:"startY"`
	EndX   int            `json:"endX"`
	EndY   int            `json:"endY"`
}

func stepCaveHandler(w http.ResponseWriter, r *http.Request) {
	var request stepRequest

	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		respondCaveError(w, "Invalid request: "+err.Error())
		return
	}

	tempFile := "current_cave_" + uuid.New().String() + ".txt"
	defer os.Remove(tempFile)

	json.NewEncoder(w).Encode(caveResponse{State: domain.StepCave(tempFile, domain.Step_t{
		State: request.State,
		Life:  request.Life,
		Death: request.Death,
	})})
}

func caveHandler(w http.ResponseWriter, r *http.Request) {
	const maxMemory = 10 << 20

	var cave domain.Cave_t

	if strings.HasPrefix(r.Header.Get("Content-Type"), "multipart/form-data") {
		r.ParseMultipartForm(maxMemory)
	} else {
		r.ParseForm()
	}

	mode := r.FormValue("mode")
	log.Printf("Mode: %s", mode)

	if mode == "random" {
		cave.State = domain.GenerateCave(r)
	} else if mode == "file" {
		cave.State = domain.LoadCave(r)
	} else {
		respondCaveError(w, "Неизвестный режим: "+mode)
		return
	}

	cave.Rows = len(cave.State)
	if cave.Rows > 0 {
		cave.Cols = len(cave.State[0])
	}

	json.NewEncoder(w).Encode(caveResponse{State: cave.State})
}

func solveMazeHandler(w http.ResponseWriter, r *http.Request) {
	var request requestSolution

	if err := json.NewDecoder(r.Body).Decode(&request); err != nil {
		respondError(w, "Invalid request: "+err.Error())
		return
	}
	tmpFile := "current_maze_" + uuid.New().String() + ".txt"

	if err := storage.SaveMazeToFile(tmpFile, request.Maze); err != nil {
		respondError(w, "Failed to save maze: "+err.Error())
		return
	}
	defer os.Remove(tmpFile)

	var response mazeResponse

	solution := domain.SolveMaze(tmpFile, domain.Solve_t{
		Method: request.Method,
		StartX: request.StartX,
		StartY: request.StartY,
		EndX:   request.EndX,
		EndY:   request.EndY,
	})

	if request.Method == 0 {
		response.SolveAlgo = solution
	} else if len(solution) > 0 {
		response.SolveML = solution[len(solution)-1]
	}

	json.NewEncoder(w).Encode(response)
}

func mazeHandler(w http.ResponseWriter, r *http.Request) {
	log.Println("Received generate request")
	const maxMemory = 10 << 20

	var state domain.Maze_t

	if strings.HasPrefix(r.Header.Get("Content-Type"), "multipart/form-data") {
		if err := r.ParseMultipartForm(maxMemory); err != nil {
			respondError(w, "Ошибка парсинга multipart формы: "+err.Error())
			return
		}
	}

	mode := r.FormValue("mode")
	log.Printf("Mode: %s", mode)

	if mode == "random" {
		state = domain.GenerateMaze(r)
	} else if mode == "file" {
		state = domain.LoadMaze(r)
	} else {
		respondError(w, "Неизвестный режим: "+mode)
		return
	}

	json.NewEncoder(w).Encode(mazeResponse{
		Floors: state.Floors,
		Walls:  state.Walls,
	})
}

func enableCORSFunc(fn http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Access-Control-Allow-Origin", "*")
		fn(w, r)
	}
}

func enableCORS(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type, Authorization")

		if r.Method == "OPTIONS" {
			return
		}

		next.ServeHTTP(w, r)
	})
}

func respondError(w http.ResponseWriter, msg string) {
	w.WriteHeader(http.StatusInternalServerError)
	json.NewEncoder(w).Encode(mazeResponse{Error: msg})
}

func respondCaveError(w http.ResponseWriter, msg string) {
	w.WriteHeader(http.StatusInternalServerError)
	json.NewEncoder(w).Encode(caveResponse{Error: msg})
}

// Handler connected frond and back and contains all small handlers
func Handler() {
	http.Handle("/", enableCORS(http.FileServer(http.Dir("./static"))))
	http.HandleFunc("/generate/maze", enableCORSFunc(mazeHandler))
	http.HandleFunc("/solve/maze", enableCORSFunc(solveMazeHandler))
	http.HandleFunc("/generate/cave", enableCORSFunc(caveHandler))
	http.HandleFunc("/step/cave", enableCORSFunc(stepCaveHandler))

	log.Println("Server started on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
