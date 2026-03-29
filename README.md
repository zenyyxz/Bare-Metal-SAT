# THEAROM: A Very Fast, Very Nerdy SAT Solver

So, my girlfriend dared me to build a theorem prover from scratch. No libraries, no shortcuts, just me and the compiler. This is the result: **THEAROM**.

It started as a simple DPLL solver (which was... fine), but then I hit the Pigeonhole Principle ($P(10, 9)$) and it took 12 seconds. I couldn't have that. So I went back into the zone, ripped out the recursion, and rebuilt it into a full-blown **CDCL (Conflict-Driven Clause Learning)** engine.

Now? $P(10, 9)$ takes **0.001s**. $P(50, 49)$—which used to be impossible—is crushed in about **90ms**.

## The Tech Stack (The "Scratch" Part)

I wanted to see how fast I could make this thing by mixing different levels of the stack:
- **C++:** Handles the high-level logic, the search loop, and the DIMACS parsing.
- **C:** Used for the core bitset and memory-mapped state tracking.
- **x86_64 Assembly:** I hand-rolled the most frequent operations (like literal satisfaction checks and SIMD searches) in NASM to squeeze out every last bit of performance.

## What’s Under the Hood?

- **CDCL + 1-UIP:** Instead of just backtracking, the solver analyzes conflicts, learns new clauses, and "backjumps" across the search tree.
- **2-Watched Literals (SIMD Optimized):** We use an AVX2-style assembly search to scan clauses for unit/conflict states. It's way faster than a linear loop.
- **LBD Clause Deletion:** We track "Literal Block Distance" to identify which learned clauses are actually useful and garbage-collect the ones that are just eating up memory.
- **Phase Saving & VSIDS:** Industrial-standard heuristics. It "remembers" successful polarities and prioritizes variables that cause the most conflicts.
- **DRAT Proofs:** If the solver says a formula is `UNSAT`, it can output a `.drat` proof file so you can actually verify it's not just a bug.

## Build & Run (Chill & Simple)

You'll need `g++`, `gcc`, and `nasm`. No `npm install` or `pip install` here.

```bash
make clean && make
```

To solve something:
```bash
./sat_solver tests/sat.cnf
```

If you want to generate a hard proof (like 50 pigeons in 49 holes) and verify it:
```bash
echo "50 49" | python3 php.py
time ./sat_solver boss_cnf.cnf proof.drat
```

Or, to generate a truly messy, random 3-SAT problem at the **phase transition** ($L/N \approx 4.26$), use the `gen_3sat.py` script:

```bash
# Generate a random 100-variable, 426-clause instance
python3 gen_3sat.py > messy.cnf

# Solve it
time ./sat_solver messy.cnf
```

## Performance Bragging Rights

- **P(10, 9):** Instant (< 1ms)
- **P(50, 49):** ~90ms
- **Random 3-SAT (Phase Transition):** ~2ms

## The Vibe

The code is a mix of high-level C++ abstractions and low-level pointer arithmetic in assembly. It’s dense, it’s fast, and it solved the dare. If you find a bug, it’s probably because I was drinking too much coffee when I wrote that specific assembly loop.

Enjoy the solver. Stay nerdy.
