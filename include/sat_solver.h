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
#include <cstdio>

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
    int lbd;

    Clause() : learned(false), activity(0), lbd(0) {}
};

struct SATSolver {
    int numVars;
    std::vector<Clause> clauses;
    std::vector<Assignment> assigns;
    std::vector<int> level;
    std::vector<int> reason;
    std::vector<int> trail;
    std::vector<int> trail_lim;
    int qhead;
    std::vector<bool> seen;
    
    // DRAT Proofs
    FILE* drat;
    
    // Restarts & Deletion
    int conflicts;
    int restart_limit;
    double restart_inc;
    int clause_limit;
    
    // Watched Literals
    std::vector<std::vector<int>> watches;

    // Heuristics
    std::vector<double> activity;
    std::vector<bool> phases;
    double var_inc;

    SATSolver() : numVars(0), drat(nullptr), conflicts(0), var_inc(1.0) {}
    ~SATSolver() { if (drat) fclose(drat); }

    bool loadDIMACS(const std::string& filename);
    bool solve();
    void printAssignment() const;
    void setDrat(const std::string& path) { drat = fopen(path.c_str(), "w"); }

private:
    bool enqueue(Literal lit, int r = -1);
    int propagate();
    void analyze(int confl, std::vector<Literal>& out_learnt, int& out_btlevel);
    void record(const std::vector<Literal>& lits);
    void cancelUntil(int level);
    void reduceDB();
    int computeLBD(const std::vector<Literal>& lits);
    
    // Preprocessing (BVE)
    void preprocess();
    bool simplify();

    int decisionLevel() const { return trail_lim.size(); }
    Literal pickBranchingLiteral();
    
    int litToIdx(Literal lit) const {
        return (lit > 0) ? (lit * 2) : ((-lit * 2) + 1);
    }
    Literal idxToLit(int idx) const {
        return (idx % 2 == 0) ? (idx / 2) : -(idx / 2);
    }
    Literal negLit(Literal lit) const { return -lit; }
};

#endif // SAT_SOLVER_H
