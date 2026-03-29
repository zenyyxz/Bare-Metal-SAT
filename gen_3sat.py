import random
import sys

# Random 3-SAT generator at phase transition
N = 100
L = 426

print(f"c Random 3-SAT generator (N={N}, L={L})")
print(f"p cnf {N} {L}")

for _ in range(L):
    lits = []
    while len(lits) < 3:
        v = random.randint(1, N)
        if v not in lits and -v not in lits:
            if random.random() < 0.5:
                v = -v
            lits.append(v)
    print(f"{lits[0]} {lits[1]} {lits[2]} 0")
