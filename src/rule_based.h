#ifndef RULE_BASED_H
#define RULE_BASED_H

#include "board.h"

#include <chrono>
#include <cstdint>
#include <utility>
#include <vector>

struct RuleBasedStats {
    long long recursiveCalls = 0;
    long long guesses = 0;
    long long backtracks = 0;

    long long singlesApplied = 0;
    long long tuplesApplied = 0;
};

class RuleBasedSolver {
public:
    bool solve(Board& board);
    bool solve(Board& board, std::chrono::milliseconds timeout);

    const RuleBasedStats& stats() const;
    bool timedOut() const;

private:
    RuleBasedStats stats_;
    bool hasDeadline_ = false;
    bool timedOut_ = false;
    std::chrono::steady_clock::time_point deadline_;

    bool solveRecursive(Board& board);
    bool timeLimitReached();

    bool applyRules(Board& board);

    bool applySingle(
        Board& board,
        bool& changed
    );

    bool applyNakedTuples(
        Board& board,
        bool& changed
    );

    bool processRegion(
        Board& board,
        const std::vector<
            std::pair<int, int>
        >& region,
        bool& changed
    );

    bool generateCombinations(
        const std::vector<int>& values,
        int combinationSize,
        int start,
        std::vector<int>& current,
    std::vector<
            std::vector<int>
        >& result
    );

    bool findBestCell(
        const Board& board,
        int& row,
        int& col
    ) const;

    int bitCount(
        std::uint64_t value
    ) const;
};

#endif
