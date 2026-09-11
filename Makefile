CXX = g++
CXXFLAGS = -O2 -Wall -Wextra -std=c++17

SRC = src/main.cpp src/board.cpp src/backtracking.cpp src/rule_based.cpp
OUT = sudoku

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
