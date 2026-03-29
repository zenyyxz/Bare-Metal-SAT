# THEAROM: A Fast SAT Solver

A high-performance SAT solver implemented from scratch in C++, C, and x86_64 Assembly. It uses a modern CDCL (Conflict-Driven Clause Learning) architecture with industrial-strength optimizations to solve complex boolean logic problems efficiently.

## Features

- **CDCL Engine:** Iterative search with 1-UIP conflict analysis and non-chronological backtracking.
- **2-Watched Literals:** Efficient unit propagation that avoids scanning every clause on every assignment.
- **VSIDS Heuristic:** Activity-based variable selection to focus on the most constrained parts of the formula.
- **Phase Saving:** Remembers the last successful polarity of variables to navigate hard Random 3-SAT problems.
- **Assembly Optimizations:** Core literal checks are hand-rolled in NASM for maximum performance.
- **Zero Dependencies:** Built entirely from scratch without external libraries.

## Performance

The solver is optimized for both structured problems (like Pigeonhole Principle) and random instances:
- **P(10, 9):** ~0.001s
- **P(50, 49):** ~81ms
- **Random 3-SAT (N=100, L=426):** ~0.002s (Phase Transition)

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

```bash
# Generate a 50-pigeon, 49-hole instance
echo "50 49" | python3 php.py

# Solve it
time ./sat_solver boss_cnf.cnf
```

### Testing with Random 3-SAT (Phase Transition)

```bash
# Generate a random 3-SAT instance at L/N = 4.26
python3 gen_3sat.py > random_3sat.cnf

# Solve it
time ./sat_solver random_3sat.cnf
```

## Project Structure

- `src/sat_solver.cpp`: The core CDCL engine and parser.
- `src/low_level.asm`: x86_64 Assembly optimizations.
- `src/bitset.c`: Custom C-based bitset for state tracking.
- `include/`: Header files for all modules.
- `tests/`: Basic SAT/UNSAT test cases.
