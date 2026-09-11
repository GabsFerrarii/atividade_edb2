# Comparação de algoritmos para resolução de Sudoku

## 1. Como compilar

1) `make`: compila o resolvedor de Sudoku.
2) `g++ -O2 -Wall -Wextra -std=c++17 scripts/graphics.cpp -o graphics`: compila o gerador de gráficos.

## 2. como executar o código
1) `sudoku`: executa a bateria ampliada e grava `results/resultados_ampliados.csv`.
2) `./graphics results/resultados.csv`: reproduz os gráficos do relatório, baseados em três puzzles nas bases 2, 3 e 4, em `results/graficos/`.
