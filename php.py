from cnfgen import PigeonholePrinciple

# Get user input
x = int(input("How many pigeons? "))
y = int(input("How many holes? "))

# Generate the formula
F = PigeonholePrinciple(x, y)

# Save to file
with open("boss_cnf.cnf", "w") as f:
    f.write(F.to_dimacs())

print("\nSuccess! The DIMACS output has been saved to boss_cnf.cnf")
