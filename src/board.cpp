#include "board.h"

#include <iomanip>
#include <stdexcept>
#include <string>

Board::Board(int base)
    : base_(base),
      size_(base * base),
      fullMask_(0),
      valid_(true),
      values_(size_, std::vector<int>(size_, 0)),
      candidates_(size_, std::vector<std::uint64_t>(size_, 0)) {

    if (base < 2) {
        throw std::invalid_argument(
            "A base deve ser pelo menos 2."
        );
    }

    if (size_ > 63) {
        throw std::invalid_argument(
            "Esta implementacao suporta no maximo 63 simbolos."
        );
    }

    fullMask_ = (std::uint64_t{1} << size_) - 1;

    for (auto& row : candidates_) {
        for (auto& mask : row) {
            mask = fullMask_;
        }
    }
}

int Board::base() const {
    return base_;
}

int Board::size() const {
    return size_;
}

int Board::cellCount() const {
    return size_ * size_;
}

int Board::value(int row, int col) const {
    return values_.at(row).at(col);
}

std::uint64_t Board::candidateMask(
    int row,
    int col
) const {
    return candidates_.at(row).at(col);
}

int Board::popcount(std::uint64_t mask) {
    return __builtin_popcountll(mask);
}

int Board::candidateCount(
    int row,
    int col
) const {
    if (value(row, col) != 0) {
        return 0;
    }

    return popcount(candidateMask(row, col));
}

std::uint64_t Board::bitFor(int value) const {
    return std::uint64_t{1} << (value - 1);
}

std::vector<int> Board::candidateValues(
    int row,
    int col
) const {
    std::vector<int> result;

    if (value(row, col) != 0) {
        return result;
    }

    const auto mask = candidateMask(row, col);

    for (int v = 1; v <= size_; ++v) {
        if (mask & bitFor(v)) {
            result.push_back(v);
        }
    }

    return result;
}

bool Board::assign(
    int row,
    int col,
    int newValue
) {
    if (!valid_ ||
        newValue < 1 ||
        newValue > size_) {

        valid_ = false;
        return false;
    }

    if (values_[row][col] != 0) {
        if (values_[row][col] == newValue) {
            return true;
        }

        valid_ = false;
        return false;
    }

    const auto bit = bitFor(newValue);

    if ((candidates_[row][col] & bit) == 0) {
        valid_ = false;
        return false;
    }

    values_[row][col] = newValue;
    candidates_[row][col] = 0;

    eliminateFromPeers(
        row,
        col,
        newValue
    );

    return valid_;
}

bool Board::eliminateCandidates(
    int row,
    int col,
    std::uint64_t mask
) {
    if (!valid_ || values_[row][col] != 0) {
        return false;
    }

    const auto before = candidates_[row][col];
    const auto after = before & ~mask;

    if (before == after) {
        return false;
    }

    candidates_[row][col] = after;

    if (after == 0) {
        valid_ = false;
    }

    return true;
}

void Board::eliminateFromPeers(
    int row,
    int col,
    int assignedValue
) {
    const auto bit = bitFor(assignedValue);

    
    for (int c = 0; c < size_; ++c) {
        if (c == col) {
            continue;
        }

        if (values_[row][c] == assignedValue) {
            valid_ = false;
            return;
        }

        if (values_[row][c] == 0) {
            eliminateCandidates(
                row,
                c,
                bit
            );

            if (!valid_) {
                return;
            }
        }
    }

    
    for (int r = 0; r < size_; ++r) {
        if (r == row) {
            continue;
        }

        if (values_[r][col] == assignedValue) {
            valid_ = false;
            return;
        }

        if (values_[r][col] == 0) {
            eliminateCandidates(
                r,
                col,
                bit
            );

            if (!valid_) {
                return;
            }
        }
    }

    
    const int startRow =
        (row / base_) * base_;

    const int startCol =
        (col / base_) * base_;

    for (
        int r = startRow;
        r < startRow + base_;
        ++r
    ) {
        for (
            int c = startCol;
            c < startCol + base_;
            ++c
        ) {
            if (r == row && c == col) {
                continue;
            }

            if (values_[r][c] == assignedValue) {
                valid_ = false;
                return;
            }

            if (values_[r][c] == 0) {
                eliminateCandidates(
                    r,
                    c,
                    bit
                );

                if (!valid_) {
                    return;
                }
            }
        }
    }
}

bool Board::isValid() const {
    return valid_;
}

bool Board::isSolved() const {
    if (!valid_) {
        return false;
    }

    for (const auto& row : values_) {
        for (int value : row) {
            if (value == 0) {
                return false;
            }
        }
    }

    return true;
}

std::vector<
    std::vector<std::pair<int, int>>
> Board::regions() const {

    std::vector<
        std::vector<std::pair<int, int>>
    > result;

    result.reserve(3 * size_);

    
    for (int r = 0; r < size_; ++r) {
        std::vector<std::pair<int, int>> region;

        for (int c = 0; c < size_; ++c) {
            region.push_back({r, c});
        }

        result.push_back(std::move(region));
    }

    
    for (int c = 0; c < size_; ++c) {
        std::vector<std::pair<int, int>> region;

        for (int r = 0; r < size_; ++r) {
            region.push_back({r, c});
        }

        result.push_back(std::move(region));
    }

    
    for (
        int boxRow = 0;
        boxRow < base_;
        ++boxRow
    ) {
        for (
            int boxCol = 0;
            boxCol < base_;
            ++boxCol
        ) {
            std::vector<std::pair<int, int>> region;

            for (
                int r = boxRow * base_;
                r < (boxRow + 1) * base_;
                ++r
            ) {
                for (
                    int c = boxCol * base_;
                    c < (boxCol + 1) * base_;
                    ++c
                ) {
                    region.push_back({r, c});
                }
            }

            result.push_back(
                std::move(region)
            );
        }
    }

    return result;
}

void Board::print(std::ostream& out) const {
    const int width =
        static_cast<int>(
            std::to_string(size_).size()
        ) + 1;

    for (int r = 0; r < size_; ++r) {
        for (int c = 0; c < size_; ++c) {
            out << std::setw(width)
                << values_[r][c];

            if (
                (c + 1) % base_ == 0 &&
                c + 1 < size_
            ) {
                out << " |";
            }
        }

        out << '\n';

        if (
            (r + 1) % base_ == 0 &&
            r + 1 < size_
        ) {
            for (
                int i = 0;
                i < size_ * width +
                    (base_ - 1) * 2;
                ++i
            ) {
                out << '-';
            }

            out << '\n';
        }
    }
}