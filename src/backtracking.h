#ifndef BACKTRACKING_H
#define BACKTRACKING_H

#include "board.h"

#include <chrono>

struct BacktrackingStats {
    long long recursiveCalls = 0;
    long long guesses = 0;
    long long backtracks = 0;
};

class BacktrackingSolver {
public:
    bool solve(Board& board);
    bool solve(Board& board, std::chrono::milliseconds timeout);

    const BacktrackingStats& stats() const;
    bool timedOut() const;

private:
    BacktrackingStats stats_;
    bool hasDeadline_ = false;
    bool timedOut_ = false;
    std::chrono::steady_clock::time_point deadline_;

    bool solveRecursive(Board& board);
    bool timeLimitReached();

    bool findBestCell(
        const Board& board,
        int& row,
        int& col
    ) const;
};

#endif
