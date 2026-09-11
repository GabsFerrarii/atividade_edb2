#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

class Board {
public:
    explicit Board(int base);

    int base() const;
    int size() const;
    int cellCount() const;

    int value(int row, int col) const;

    std::uint64_t candidateMask(int row, int col) const;
    int candidateCount(int row, int col) const;
    std::vector<int> candidateValues(int row, int col) const;

    bool assign(int row, int col, int value);
    bool eliminateCandidates(
        int row,
        int col,
        std::uint64_t mask
    );

    bool isValid() const;
    bool isSolved() const;

    std::vector<std::vector<std::pair<int, int>>> regions() const;

    void print(std::ostream& out = std::cout) const;

private:
    int base_;
    int size_;

    std::uint64_t fullMask_;
    bool valid_;

    std::vector<std::vector<int>> values_;
    std::vector<std::vector<std::uint64_t>> candidates_;

    static int popcount(std::uint64_t mask);

    std::uint64_t bitFor(int value) const;

    void eliminateFromPeers(
        int row,
        int col,
        int value
    );
};

#endif