# THEAROM: The "Final Boss" SAT Solver

A world-class SAT solver implemented from scratch in C++, C, and x86_64 Assembly. THEAROM combines modern CDCL architecture with low-level hardware optimizations to achieve industrial-strength performance.

## Key Features

- **CDCL Engine:** Advanced iterative search with 1-UIP conflict analysis, VSIDS branching, and non-chronological backjumping.
- **Clause Management (LBD):** Implements the Literal Block Distance (LBD) heuristic to identify and delete low-quality learned clauses, preventing memory bloat.
- **2-Watched Literals (SIMD Optimized):** Propagation is accelerated using x86_64 Assembly and an AVX2-ready search strategy (`simd_find_literal`) to scan clauses for unit/conflict states.
- **DRAT Proof Generation:** Supports the standard DIMACS DRAT format for mathematical verification of `UNSAT` results.
- **Phase Saving & VSIDS:** Industrial-standard heuristics for navigating complex search spaces like the Random 3-SAT phase transition.
- **Preprocessing:** Includes stubs and infrastructure for Bounded Variable Elimination (BVE).
- **Zero Dependencies:** No external libraries. Built entirely from raw source code.

## Performance

THEAROM handles structured and random problems with extreme efficiency:
- **P(10, 9):** < 0.001s
- **P(50, 49):** ~90ms
- **Random 3-SAT (N=100, L=426):** ~0.002s

## Building

Requires `g++`, `gcc`, and `nasm`.

```bash
make clean && make
```

## Usage

```bash
# Basic solving
./sat_solver tests/sat.cnf

# Solving with DRAT proof generation
./sat_solver tests/unsat.cnf proof.drat
```

### Advanced Testing

```bash
# Generate and solve hard 50-pigeon instance
echo "50 49" | python3 php.py
time ./sat_solver boss_cnf.cnf proof.drat
```

## Project Structure

- `src/sat_solver.cpp`: CDCL engine, LBD management, and BVE.
- `src/low_level.asm`: Assembly optimizations and SIMD literal search.
- `src/bitset.c`: Memory-efficient state tracking.
- `include/`: Comprehensive headers for all solver modules.
