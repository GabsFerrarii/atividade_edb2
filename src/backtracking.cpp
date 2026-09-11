#include "backtracking.h"

#include <utility>
#include <vector>

bool BacktrackingSolver::solve(Board& board) {
    stats_ = {};
    hasDeadline_ = false;
    timedOut_ = false;

    if (!board.isValid()) {
        return false;
    }

    return solveRecursive(board);
}

bool BacktrackingSolver::solve(
    Board& board,
    std::chrono::milliseconds timeout
) {
    stats_ = {};
    hasDeadline_ = true;
    timedOut_ = false;
    deadline_ = std::chrono::steady_clock::now() + timeout;

    if (!board.isValid()) {
        return false;
    }

    return solveRecursive(board);
}

const BacktrackingStats&
BacktrackingSolver::stats() const {
    return stats_;
}

bool BacktrackingSolver::timedOut() const {
    return timedOut_;
}

bool BacktrackingSolver::solveRecursive(
    Board& board
) {
    if (timeLimitReached()) {
        return false;
    }

    ++stats_.recursiveCalls;

    if (!board.isValid()) {
        return false;
    }

    if (board.isSolved()) {
        return true;
    }

    int row = -1;
    int col = -1;

    if (!findBestCell(board, row, col)) {
        return false;
    }

    const std::vector<int> candidates =
        board.candidateValues(row, col);

    
    if (candidates.empty()) {
        return false;
    }

    
    for (int value : candidates) {
        ++stats_.guesses;

        
        Board attempt = board;

        if (!attempt.assign(
                row,
                col,
                value
            )) {

            ++stats_.backtracks;
            continue;
        }

        
        if (solveRecursive(attempt)) {
            board = std::move(attempt);

            return true;
        }

        
        ++stats_.backtracks;
    }

    return false;
}

bool BacktrackingSolver::findBestCell(
    const Board& board,
    int& row,
    int& col
) const {
    int smallestCandidateCount =
        board.size() + 1;

    bool found = false;

    for (int r = 0; r < board.size(); ++r) {
        for (int c = 0; c < board.size(); ++c) {

            if (board.value(r, c) != 0) {
                continue;
            }

            found = true;

            const int count =
                board.candidateCount(r, c);

            
            if (count == 0) {
                row = r;
                col = c;

                return true;
            }

            
            if (count < smallestCandidateCount) {
                smallestCandidateCount = count;

                row = r;
                col = c;
            }
        }
    }

    return found;
}

bool BacktrackingSolver::timeLimitReached() {
    if (
        hasDeadline_ &&
        std::chrono::steady_clock::now() >= deadline_
    ) {
        timedOut_ = true;
        return true;
    }

    return false;
}
