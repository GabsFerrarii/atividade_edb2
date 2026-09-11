from __future__ import annotations

import argparse
import csv
import hashlib
import random
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
INPUTS = ROOT / "inputs"
BASES = (2, 3, 4, 5, 6, 7)
SEEDS = {
    2: {4: 2026090204, 5: 2026090205},
    3: {4: 2026090304, 5: 2026090305},
    4: {4: 2026090404, 5: 2026090405},
    5: {4: 2026090504, 5: 2026090505},
    6: {1: 2026090601, 2: 2026090602, 3: 2026090603, 4: 2026090604, 5: 2026090605},
    7: {1: 2026090701, 2: 2026090702, 3: 2026090703, 4: 2026090704, 5: 2026090705},
}


def canonical_solution(base: int) -> list[list[int]]:
    side = base * base
    return [
        [((row * base + row // base + col) % side) + 1 for col in range(side)]
        for row in range(side)
    ]


def permuted_solution(base: int, seed: int) -> list[list[int]]:
    side = base * base
    rng = random.Random(seed)
    symbols = list(range(1, side + 1))
    rng.shuffle(symbols)

    bands = list(range(base))
    rng.shuffle(bands)
    rows = []
    for band in bands:
        within_band = list(range(base))
        rng.shuffle(within_band)
        rows.extend(band * base + offset for offset in within_band)

    stacks = list(range(base))
    rng.shuffle(stacks)
    columns = []
    for stack in stacks:
        within_stack = list(range(base))
        rng.shuffle(within_stack)
        columns.extend(stack * base + offset for offset in within_stack)

    canonical = canonical_solution(base)
    return [[symbols[canonical[row][col] - 1] for col in columns] for row in rows]


def puzzle_from_solution(solution: list[list[int]], seed: int) -> list[list[int]]:
    side = len(solution)
    puzzle = [row[:] for row in solution]
    empty_cells = (side * side + 1) // 2
    rng = random.Random(seed ^ 0x5EED)
    for position in rng.sample(range(side * side), empty_cells):
        puzzle[position // side][position % side] = 0
    return puzzle


def read_puzzle(path: Path, side: int) -> list[list[int]]:
    values = [int(value) for value in path.read_text(encoding="utf-8").split()]
    if len(values) != side * side:
        raise ValueError(f"{path}: esperado {side * side} valores, encontrado {len(values)}")
    return [values[index:index + side] for index in range(0, len(values), side)]


def valid_grid(grid: list[list[int]], base: int, allow_zero: bool) -> bool:
    side = base * base
    expected = set(range(1, side + 1))
    for index in range(side):
        row = [value for value in grid[index] if value != 0]
        column = [grid[row_index][index] for row_index in range(side) if grid[row_index][index] != 0]
        if any(value not in expected for value in row + column):
            return False
        if len(row) != len(set(row)) or len(column) != len(set(column)):
            return False
    for start_row in range(0, side, base):
        for start_col in range(0, side, base):
            box = [
                grid[row][col]
                for row in range(start_row, start_row + base)
                for col in range(start_col, start_col + base)
                if grid[row][col] != 0
            ]
            if any(value not in expected for value in box) or len(box) != len(set(box)):
                return False
    if not allow_zero and any(0 in row for row in grid):
        return False
    return True


def write_puzzle(path: Path, puzzle: list[list[int]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(" ".join(map(str, row)) for row in puzzle) + "\n", encoding="utf-8")


def generated_puzzle(base: int, puzzle_number: int) -> list[list[int]]:
    seed = SEEDS[base][puzzle_number]
    solution = permuted_solution(base, seed)
    if not valid_grid(solution, base, allow_zero=False):
        raise ValueError(f"solucao gerada invalida: base {base}, sudoku{puzzle_number}")
    puzzle = puzzle_from_solution(solution, seed)
    if not valid_grid(puzzle, base, allow_zero=True):
        raise ValueError(f"puzzle gerado invalido: base {base}, sudoku{puzzle_number}")
    return puzzle


def should_preserve(base: int, puzzle_number: int, path: Path) -> bool:
    return base <= 5 and puzzle_number <= 3 and path.exists()


def create_inputs() -> None:
    for base in BASES:
        side = base * base
        for puzzle_number in range(1, 6):
            path = INPUTS / f"{side}x{side}" / f"sudoku{puzzle_number}.txt"
            if should_preserve(base, puzzle_number, path):
                continue
            write_puzzle(path, generated_puzzle(base, puzzle_number))


def write_manifest() -> None:
    manifest = INPUTS / "seeds.csv"
    with manifest.open("w", newline="", encoding="utf-8") as output:
        writer = csv.writer(output)
        writer.writerow(["base", "lado", "puzzle", "seed", "vazias", "celulas", "origem", "sha256"])
        for base in BASES:
            side = base * base
            for puzzle_number in range(1, 6):
                path = INPUTS / f"{side}x{side}" / f"sudoku{puzzle_number}.txt"
                puzzle = read_puzzle(path, side)
                seed = SEEDS.get(base, {}).get(puzzle_number, "")
                origin = "gerado" if seed else "existente_preservado"
                content = path.read_bytes()
                writer.writerow([
                    base,
                    side,
                    f"sudoku{puzzle_number}",
                    seed,
                    sum(value == 0 for row in puzzle for value in row),
                    side * side,
                    origin,
                    hashlib.sha256(content).hexdigest(),
                ])


def verify_inputs() -> None:
    for base in BASES:
        side = base * base
        for puzzle_number in range(1, 6):
            path = INPUTS / f"{side}x{side}" / f"sudoku{puzzle_number}.txt"
            puzzle = read_puzzle(path, side)
            if not valid_grid(puzzle, base, allow_zero=True):
                raise ValueError(f"puzzle invalido: {path}")
            if puzzle_number in SEEDS.get(base, {}):
                expected = generated_puzzle(base, puzzle_number)
                if puzzle != expected:
                    raise ValueError(f"puzzle nao reproduz a seed registrada: {path}")
    print("Todos os 30 puzzles sao validos; os puzzles gerados reproduzem as seeds registradas.")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--verify", action="store_true")
    args = parser.parse_args()
    if not args.verify:
        create_inputs()
        write_manifest()
    verify_inputs()


if __name__ == "__main__":
    main()
