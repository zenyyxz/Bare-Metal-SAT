# THEAROM: A Fast SAT Solver

A high-performance SAT solver implemented from scratch in C++, C, and x86_64 Assembly. It uses a modern CDCL (Conflict-Driven Clause Learning) architecture to solve complex boolean logic problems efficiently.

## Features

- **CDCL Engine:** Iterative search with 1-UIP conflict analysis and non-chronological backtracking.
- **2-Watched Literals:** Efficient unit propagation that avoids scanning every clause on every assignment.
- **VSIDS Heuristic:** Activity-based variable selection to focus on the most constrained parts of the formula.
- **Assembly Optimizations:** Core literal checks are hand-rolled in NASM for maximum performance.
- **Zero Dependencies:** Built entirely from scratch without external libraries.

## Performance

The solver is optimized for Pigeonhole Principle (PHP) problems and other hard SAT instances:
- **P(10, 9):** ~0.001s
- **P(50, 49):** ~81ms

## Building

You'll need `g++`, `gcc`, and `nasm` installed.

```bash
make clean && make
```

## Usage

The solver accepts standard DIMACS CNF files.

```bash
./sat_solver tests/sat.cnf
```

### Testing with Pigeonhole Principle

You can use the included `php.py` script to generate hard test cases:

```bash
# Generate a 50-pigeon, 49-hole instance
echo "50 49" | python3 php.py

# Solve it
time ./sat_solver boss_cnf.cnf
```

## Project Structure

- `src/sat_solver.cpp`: The core CDCL engine and parser.
- `src/low_level.asm`: x86_64 Assembly optimizations.
- `src/bitset.c`: Custom C-based bitset for state tracking.
- `include/`: Header files for all modules.
- `tests/`: Basic SAT/UNSAT test cases.
