#include "backtracking.h"
#include "board.h"
#include "rule_based.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

constexpr int repetitions = 10;
std::chrono::seconds largeBaseTimeout(30);

Board loadBoard(const std::string& path, int base) {
    Board board(base);
    std::ifstream input(path);

    if (!input) {
        throw std::runtime_error("Nao foi possivel abrir " + path + ".");
    }

    for (int row = 0; row < board.size(); ++row) {
        for (int col = 0; col < board.size(); ++col) {
            int value = 0;

            if (!(input >> value)) {
                throw std::runtime_error("Arquivo de Sudoku incompleto: " + path);
            }

            if (value != 0 && !board.assign(row, col, value)) {
                throw std::runtime_error("Sudoku invalido: " + path);
            }
        }
    }

    return board;
}

void writeBacktrackingRows(
    const Board& original,
    int base,
    const std::string& puzzle,
    bool useTimeout,
    std::ofstream& csv
) {
    {
        Board board = original;
        BacktrackingSolver solver;

        if (useTimeout) {
            solver.solve(board, largeBaseTimeout);
        } else {
            solver.solve(board);
        }
    }

    for (int repetition = 1; repetition <= repetitions; ++repetition) {
        Board board = original;
        BacktrackingSolver solver;

        const auto start = std::chrono::steady_clock::now();
        const bool solved = useTimeout
            ? solver.solve(board, largeBaseTimeout)
            : solver.solve(board);
        const auto end = std::chrono::steady_clock::now();

        const double microseconds =
            std::chrono::duration<double, std::micro>(end - start).count();
        const double milliseconds =
            std::chrono::duration<double, std::milli>(end - start).count();
        const BacktrackingStats& stats = solver.stats();
        const std::string status = solver.timedOut()
            ? "timeout"
            : (solved && board.isSolved() ? "solved" : "unsolved");

        csv << base << ','
            << board.size() << ','
            << board.cellCount() << ','
            << puzzle << ",backtracking,"
            << repetition << ','
            << microseconds << ','
            << milliseconds << ','
            << stats.recursiveCalls << ','
            << stats.guesses << ','
            << stats.backtracks << ",0,0,"
            << status << '\n';

        if (status == "timeout") {
            return;
        }

        if (status != "solved") {
            throw std::runtime_error("Backtracking nao encontrou solucao para " + puzzle);
        }
    }
}

void writeRuleBasedRows(
    const Board& original,
    int base,
    const std::string& puzzle,
    bool useTimeout,
    std::ofstream& csv
) {
    {
        Board board = original;
        RuleBasedSolver solver;

        if (useTimeout) {
            solver.solve(board, largeBaseTimeout);
        } else {
            solver.solve(board);
        }
    }

    for (int repetition = 1; repetition <= repetitions; ++repetition) {
        Board board = original;
        RuleBasedSolver solver;

        const auto start = std::chrono::steady_clock::now();
        const bool solved = useTimeout
            ? solver.solve(board, largeBaseTimeout)
            : solver.solve(board);
        const auto end = std::chrono::steady_clock::now();

        const double microseconds =
            std::chrono::duration<double, std::micro>(end - start).count();
        const double milliseconds =
            std::chrono::duration<double, std::milli>(end - start).count();
        const RuleBasedStats& stats = solver.stats();
        const std::string status = solver.timedOut()
            ? "timeout"
            : (solved && board.isSolved() ? "solved" : "unsolved");

        csv << base << ','
            << board.size() << ','
            << board.cellCount() << ','
            << puzzle << ",rule_based,"
            << repetition << ','
            << microseconds << ','
            << milliseconds << ','
            << stats.recursiveCalls << ','
            << stats.guesses << ','
            << stats.backtracks << ','
            << stats.singlesApplied << ','
            << stats.tuplesApplied << ','
            << status << '\n';

        if (status == "timeout") {
            return;
        }

        if (status != "solved") {
            throw std::runtime_error("Rule-based nao encontrou solucao para " + puzzle);
        }
    }
}

void runPuzzle(
    int base,
    const std::string& puzzle,
    bool useTimeout,
    const std::string& inputRoot,
    std::ofstream& csv
) {
    const int side = base * base;
    const std::string path = inputRoot + "/" + std::to_string(side) + "x" +
        std::to_string(side) + "/" + puzzle + ".txt";
    const Board original = loadBoard(path, base);

    writeBacktrackingRows(original, base, puzzle, useTimeout, csv);
    writeRuleBasedRows(original, base, puzzle, useTimeout, csv);
}

void probePuzzle(
    int base,
    const std::string& puzzle,
    std::ofstream& csv,
    const std::string& inputRoot
) {
    const int side = base * base;
    const std::string path = inputRoot + "/" + std::to_string(side) + "x" +
        std::to_string(side) + "/" + puzzle + ".txt";
    const Board original = loadBoard(path, base);

    {
        Board board = original;
        BacktrackingSolver solver;
        const auto start = std::chrono::steady_clock::now();
        const bool solved = solver.solve(board, largeBaseTimeout);
        const auto end = std::chrono::steady_clock::now();
        const std::string status = solver.timedOut()
            ? "timeout"
            : (solved && board.isSolved() ? "solved" : "unsolved");
        const double microseconds =
            std::chrono::duration<double, std::micro>(end - start).count();
        const double milliseconds =
            std::chrono::duration<double, std::milli>(end - start).count();

        csv << base << ',' << side << ',' << side * side << ','
            << puzzle << ",backtracking," << microseconds << ','
            << milliseconds << ',' << status << '\n';
        std::cout << base << ',' << puzzle << ",backtracking,"
                  << milliseconds << ',' << status << '\n';
    }

    {
        Board board = original;
        RuleBasedSolver solver;
        const auto start = std::chrono::steady_clock::now();
        const bool solved = solver.solve(board, largeBaseTimeout);
        const auto end = std::chrono::steady_clock::now();
        const std::string status = solver.timedOut()
            ? "timeout"
            : (solved && board.isSolved() ? "solved" : "unsolved");
        const double microseconds =
            std::chrono::duration<double, std::micro>(end - start).count();
        const double milliseconds =
            std::chrono::duration<double, std::milli>(end - start).count();

        csv << base << ',' << side << ',' << side * side << ','
            << puzzle << ",rule_based," << microseconds << ','
            << milliseconds << ',' << status << '\n';
        std::cout << base << ',' << puzzle << ",rule_based,"
                  << milliseconds << ',' << status << '\n';
    }
}

int main(int argc, char* argv[]) {
    try {
        const std::string inputRoot = "inputs";
        bool probe = false;

        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];

            if (argument == "--probe") {
                probe = true;
                continue;
            }

            if (argument == "--timeout-seconds" && index + 1 < argc) {
                const long long seconds = std::stoll(argv[++index]);

                if (seconds <= 0) {
                    throw std::runtime_error("O timeout deve ser positivo.");
                }

                largeBaseTimeout = std::chrono::seconds(seconds);
                continue;
            }

            throw std::runtime_error(
                "Uso: sudoku [--probe] [--timeout-seconds N]"
            );
        }

        if (probe) {
            const std::string probePath = "results/sondagem_ampliada.csv";
            std::ofstream csv(probePath);

            if (!csv) {
                throw std::runtime_error("Nao foi possivel criar " + probePath + ".");
            }

            csv << "base,lado,celulas,puzzle,algoritmo,tempo_us,tempo_ms,status\n";
            csv << std::fixed << std::setprecision(6);
            std::cout << "base,puzzle,algoritmo,tempo_ms,status\n";

            for (int base = 5; base <= 7; ++base) {
                for (int puzzleNumber = 1; puzzleNumber <= 5; ++puzzleNumber) {
                    probePuzzle(
                        base,
                        "sudoku" + std::to_string(puzzleNumber),
                        csv,
                        inputRoot
                    );
                }
            }

            std::cout << "Sondagem salva em " << probePath << '\n';
            return 0;
        }

        const std::string outputPath = "results/resultados_ampliados.csv";
        std::ofstream csv(outputPath);

        if (!csv) {
            throw std::runtime_error("Nao foi possivel criar " + outputPath + ".");
        }

        csv << "base,lado,celulas,puzzle,algoritmo,repeticao,tempo_us,tempo_ms,"
            << "recursive_calls,guesses,backtracks,singles,tuples,status\n";
        csv << std::fixed << std::setprecision(6);

        for (int base = 2; base <= 7; ++base) {
            for (int puzzleNumber = 1; puzzleNumber <= 5; ++puzzleNumber) {
                runPuzzle(
                    base,
                    "sudoku" + std::to_string(puzzleNumber),
                    base >= 5,
                    inputRoot,
                    csv
                );
            }
        }

        std::cout << "Resultados salvos em " << outputPath << '\n';
        std::cout << "Bases 5 a 7 usam limite de " << largeBaseTimeout.count()
                  << " s por repeticao.\n";
    }
    catch (const std::exception& error) {
        std::cerr << "Erro: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
