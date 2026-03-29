# THEAROM: From Logic Solver to Logic Predator

She challenged me. I built a DPLL solver. It was okay. It solved things. But then I hit the Pigeonhole Principle ($P(10, 9)$), and the machine started sweating. 12 seconds. That wasn't good enough.

So I went back into the cellar. I ripped out the recursive heart of the solver and replaced it with a **CDCL (Conflict-Driven Clause Learning)** engine.

Now? $P(10, 9)$ takes **0.001s**.
$P(50, 49)$—a problem that would have taken the old version *literally forever*—is crushed in **81 milliseconds**.

## The Evolution: DPLL to CDCL

The old version (DPLL) was like a blind man in a maze, backtracking every time he hit a wall.
The new version (CDCL) is like a cartographer. Every time it hits a wall, it learns *why* that wall exists, writes a new rule (Clause Learning) to never go there again, and jumps back to the last meaningful decision point (Backjumping).

### The Technical Stack (Still Zero Libraries)

*   **CDCL Engine (C++):** Iterative search, 1-UIP conflict analysis, and VSIDS branching. No recursion overhead.
*   **2-Watched Literals (C++):** An industrial-strength propagation technique. We don't scan clauses anymore; we watch them.
*   **Bitset Logic (C):** Custom memory-mapped bitsets for fast state tracking.
*   **The Assembly Heart (x86_64):** The most frequent operations—checking if a literal is satisfied or falsified—are still hand-rolled in NASM. Because even with a smart algorithm, you still need raw power.

## How to Run

```bash
make clean && make
# Generate a hard problem (e.g., 50 pigeons, 49 holes)
python3 php.py
# Watch it disappear
time ./sat_solver boss_cnf.cnf
```

## The "Human" Part

I spent all night on the conflict analysis logic. It's ugly, it's dense, and it works. There are probably a few "dev-only" comments left in the source code. That's fine. It's not academic code anymore; it's code that wins dares.

— *A coder who just won a very expensive dinner.*
