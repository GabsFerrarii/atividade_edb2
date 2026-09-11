#include "rule_based.h"

#include <algorithm>
#include <utility>

bool RuleBasedSolver::solve(Board& board) {
    stats_ = {};
    hasDeadline_ = false;
    timedOut_ = false;

    if (!board.isValid()) {
        return false;
    }

    return solveRecursive(board);
}

bool RuleBasedSolver::solve(
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

const RuleBasedStats&
RuleBasedSolver::stats() const {
    return stats_;
}

bool RuleBasedSolver::timedOut() const {
    return timedOut_;
}

bool RuleBasedSolver::solveRecursive(
    Board& board
) {
    if (timeLimitReached()) {
        return false;
    }

    ++stats_.recursiveCalls;

    if (!board.isValid()) {
        return false;
    }

    
    if (!applyRules(board)) {
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

bool RuleBasedSolver::applyRules(
    Board& board
) {
    while (
        board.isValid() &&
        !board.isSolved()
    ) {
        if (timeLimitReached()) {
            return false;
        }

        bool changed = false;

        
        if (!applySingle(
                board,
                changed
            )) {

            return false;
        }

        if (changed) {
            continue;
        }

        
        if (!applyNakedTuples(
                board,
                changed
            )) {

            return false;
        }

        if (changed) {
            continue;
        }

        
        break;
    }

    return board.isValid();
}

bool RuleBasedSolver::applySingle(
    Board& board,
    bool& changed
) {
    changed = false;

    for (int row = 0;
         row < board.size();
         ++row) {

        for (int col = 0;
             col < board.size();
             ++col) {

            if (board.value(row, col) != 0) {
                continue;
            }

            const int count =
                board.candidateCount(
                    row,
                    col
                );

            
            if (count == 0) {
                return false;
            }

            
            if (count == 1) {
                const auto candidates =
                    board.candidateValues(
                        row,
                        col
                    );

                const int value =
                    candidates.front();

                if (!board.assign(
                        row,
                        col,
                        value
                    )) {

                    return false;
                }

                ++stats_.singlesApplied;

                changed = true;

                
                return true;
            }
        }
    }

    return true;
}

bool RuleBasedSolver::applyNakedTuples(
    Board& board,
    bool& changed
) {
    changed = false;

    const auto allRegions =
        board.regions();

    for (const auto& region : allRegions) {
        if (timeLimitReached()) {
            return false;
        }

        if (!processRegion(
                board,
                region,
                changed
            )) {

            return false;
        }

        if (changed) {
            return true;
        }
    }

    return true;
}

bool RuleBasedSolver::processRegion(
    Board& board,
    const std::vector<
        std::pair<int, int>
    >& region,
    bool& changed
) {
    
    std::vector<int> emptyCells;

    for (int i = 0;
         i < static_cast<int>(region.size());
         ++i) {

        const auto [row, col] =
            region[i];

        if (board.value(row, col) == 0) {
            emptyCells.push_back(i);
        }
    }

    
    const int maximumTuple =
        std::min(
            board.size() - 1,
            static_cast<int>(
                emptyCells.size()
            )
        );

    for (
        int tupleSize = 2;
        tupleSize <= maximumTuple;
        ++tupleSize
    ) {
        if (timeLimitReached()) {
            return false;
        }

        std::vector<
            std::vector<int>
        > combinations;

        std::vector<int> current;

        if (!generateCombinations(
            emptyCells,
            tupleSize,
            0,
            current,
            combinations
        )) {
            return false;
        }

        for (const auto& combination :
             combinations) {
            if (timeLimitReached()) {
                return false;
            }

            std::uint64_t unionMask = 0;

            
            for (int index : combination) {

                const auto [row, col] =
                    region[index];

                unionMask |=
                    board.candidateMask(
                        row,
                        col
                    );
            }

            const int differentCandidates =
                bitCount(unionMask);

            
            if (
                differentCandidates
                != tupleSize
            ) {
                continue;
            }

            bool eliminated = false;

            
            for (int index : emptyCells) {

                if (
                    std::find(
                        combination.begin(),
                        combination.end(),
                        index
                    )
                    != combination.end()
                ) {
                    continue;
                }

                const auto [row, col] =
                    region[index];

                if (
                    board.eliminateCandidates(
                        row,
                        col,
                        unionMask
                    )
                ) {
                    eliminated = true;

                    if (!board.isValid()) {
                        return false;
                    }
                }
            }

            if (eliminated) {
                ++stats_.tuplesApplied;

                changed = true;

                return true;
            }
        }
    }

    return true;
}

bool RuleBasedSolver::generateCombinations(
    const std::vector<int>& values,
    int combinationSize,
    int start,
    std::vector<int>& current,
    std::vector<
        std::vector<int>
    >& result
) {
    if (timeLimitReached()) {
        return false;
    }

    
    if (
        static_cast<int>(
            current.size()
        ) == combinationSize
    ) {
        result.push_back(current);
        return true;
    }

    const int missing =
        combinationSize -
        static_cast<int>(
            current.size()
        );

    for (
        int i = start;
        i <=
            static_cast<int>(
                values.size()
            ) - missing;
        ++i
    ) {
        current.push_back(
            values[i]
        );

        if (!generateCombinations(
            values,
            combinationSize,
            i + 1,
            current,
            result
        )) {
            return false;
        }

        current.pop_back();
    }

    return true;
}

bool RuleBasedSolver::findBestCell(
    const Board& board,
    int& row,
    int& col
) const {
    int smallestCandidateCount =
        board.size() + 1;

    bool found = false;

    for (int r = 0;
         r < board.size();
         ++r) {

        for (int c = 0;
             c < board.size();
             ++c) {

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

            if (
                count <
                smallestCandidateCount
            ) {
                smallestCandidateCount =
                    count;

                row = r;
                col = c;
            }
        }
    }

    return found;
}

int RuleBasedSolver::bitCount(
    std::uint64_t value
) const {
    return __builtin_popcountll(value);
}

bool RuleBasedSolver::timeLimitReached() {
    if (
        hasDeadline_ &&
        std::chrono::steady_clock::now() >= deadline_
    ) {
        timedOut_ = true;
        return true;
    }

    return false;
}
