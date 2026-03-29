#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <map>
#include <cmath>

enum class Assignment {
    UNASSIGNED = 0,
    TRUE = 1,
    FALSE = 2
};

typedef int Literal;

struct Clause {
    std::vector<Literal> lits;
    bool learned;
    int activity;

    Clause() : learned(false), activity(0) {}
};

struct SATSolver {
    int numVars;
    std::vector<Clause> clauses;
    std::vector<Assignment> assigns;
    std::vector<int> level;
    std::vector<int> reason; // Index of clause that forced this assignment
    std::vector<int> trail;
    std::vector<int> trail_lim;
    int qhead;
    std::vector<bool> seen;
    
    // Restarts
    int restarts;
    double restart_inc;
    int restart_limit;
    
    // Watched Literals
    std::vector<std::vector<int>> watches; // literal -> list of clause indices

    // Heuristics
    std::vector<double> activity;
    double var_inc;

    SATSolver() : numVars(0), var_inc(1.0) {}

    bool loadDIMACS(const std::string& filename);
    bool solve();
    void printAssignment() const;

private:
    bool enqueue(Literal lit, int r = -1);
    int propagate();
    void analyze(int confl, std::vector<Literal>& out_learnt, int& out_btlevel);
    void record(const std::vector<Literal>& lits);
    void cancelUntil(int level);
    
    int decisionLevel() const { return trail_lim.size(); }
    Literal pickBranchingLiteral();
    
    // Literal helper
    int litToIdx(Literal lit) const {
        return (lit > 0) ? (lit * 2) : ((-lit * 2) + 1);
    }
    Literal idxToLit(int idx) const {
        return (idx % 2 == 0) ? (idx / 2) : -(idx / 2);
    }
    Literal negLit(Literal lit) const { return -lit; }
};

#endif // SAT_SOLVER_H
