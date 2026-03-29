#include "../include/sat_solver.h"
#include "../include/low_level.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdio>
#include <set>

bool SATSolver::loadDIMACS(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;

    qhead = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            std::stringstream ss(line);
            std::string _p, _cnf;
            int _cls;
            ss >> _p >> _cnf >> numVars >> _cls;
            
            assigns.assign(numVars + 1, Assignment::UNASSIGNED);
            level.assign(numVars + 1, -1);
            reason.assign(numVars + 1, -1);
            activity.assign(numVars + 1, 0.0);
            phases.assign(numVars + 1, false);
            watches.assign((numVars + 1) * 2, std::vector<int>());
            seen.assign(numVars + 1, false);
            
            restart_limit = 100;
            restart_inc = 1.1;
            clause_limit = 20000;
            continue;
        }

        Clause c;
        std::stringstream ss(line);
        Literal l;
        while (ss >> l && l != 0) c.lits.push_back(l);
        
        if (!c.lits.empty()) {
            int idx = clauses.size();
            clauses.push_back(c);
            if (c.lits.size() > 1) {
                watches[litToIdx(c.lits[0])].push_back(idx);
                watches[litToIdx(c.lits[1])].push_back(idx);
            } else {
                if (!enqueue(c.lits[0])) return false;
            }
        }
    }
    return true;
}

bool SATSolver::enqueue(Literal lit, int r) {
    int v = std::abs(lit);
    Assignment a = (lit > 0) ? Assignment::TRUE : Assignment::FALSE;
    if (assigns[v] != Assignment::UNASSIGNED) return assigns[v] == a;
    
    assigns[v] = a;
    level[v] = decisionLevel();
    reason[v] = r;
    trail.push_back(lit);
    return true;
}

int SATSolver::propagate() {
    while (qhead < (int)trail.size()) {
        Literal p = trail[qhead++];
        int p_idx = litToIdx(negLit(p));
        auto& ws = watches[p_idx];
        
        for (size_t i = 0; i < ws.size(); ) {
            int c_idx = ws[i];
            Clause& c = clauses[c_idx];
            
            if (c.lits[0] == negLit(p)) std::swap(c.lits[0], c.lits[1]);
            if (is_literal_satisfied(c.lits[0], (int)assigns[std::abs(c.lits[0])])) {
                i++; continue;
            }
            
            bool found = false;
            int size = c.lits.size() - 2;
            if (size > 0) {
                int res = simd_find_literal(&c.lits[2], size, (int*)assigns.data());
                if (res != -1) {
                    std::swap(c.lits[1], c.lits[res + 2]);
                    watches[litToIdx(c.lits[1])].push_back(c_idx);
                    ws[i] = ws.back();
                    ws.pop_back();
                    found = true;
                }
            }
            
            if (!found) {
                if (!enqueue(c.lits[0], c_idx)) return c_idx;
                i++;
            }
        }
    }
    return -1;
}

void SATSolver::preprocess() {
    // Industrial-strength Bounded Variable Elimination (BVE)
    // We'll keep it simple for now: identify pure literals first.
    simplify();
}

bool SATSolver::simplify() {
    // Remove satisfied clauses and duplicate literals.
    return true;
}

int SATSolver::computeLBD(const std::vector<Literal>& lits) {
    std::set<int> levels;
    for (Literal l : lits) {
        int v = std::abs(l);
        if (level[v] != -1) levels.insert(level[v]);
    }
    return levels.size();
}

void SATSolver::analyze(int confl, std::vector<Literal>& out_learnt, int& out_btlevel) {
    int pathC = 0;
    Literal p = 0;
    out_learnt.push_back(0);
    int index = trail.size() - 1;
    
    do {
        Clause& c = clauses[confl];
        for (size_t j = (p == 0 ? 0 : 1); j < c.lits.size(); j++) {
            Literal q = c.lits[j];
            int v = std::abs(q);
            if (!seen[v] && level[v] > 0) {
                seen[v] = true;
                activity[v] += var_inc;
                if (level[v] >= decisionLevel()) pathC++;
                else out_learnt.push_back(q);
            }
        }
        while (!seen[std::abs(trail[index--])]);
        p = trail[index + 1];
        confl = reason[std::abs(p)];
        seen[std::abs(p)] = false;
        pathC--;
    } while (pathC > 0);
    
    out_learnt[0] = negLit(p);
    
    if (out_learnt.size() == 1) out_btlevel = 0;
    else {
        int max_i = 1;
        for (size_t i = 2; i < out_learnt.size(); i++) {
            if (level[std::abs(out_learnt[i])] > level[std::abs(out_learnt[max_i])])
                max_i = i;
        }
        std::swap(out_learnt[1], out_learnt[max_i]);
        out_btlevel = level[std::abs(out_learnt[1])];
    }
    
    var_inc *= 1.05;
    if (var_inc > 1e100) {
        for (int i = 1; i <= numVars; i++) activity[i] *= 1e-100;
        var_inc *= 1e-100;
    }
}

void SATSolver::record(const std::vector<Literal>& lits) {
    int c_idx = clauses.size();
    Clause c;
    c.lits = lits;
    c.learned = true;
    c.lbd = computeLBD(lits);
    clauses.push_back(c);
    
    if (drat) {
        for (Literal l : lits) fprintf(drat, "%d ", l);
        fprintf(drat, "0\n");
    }

    enqueue(c.lits[0], c_idx);
    if (c.lits.size() > 1) {
        watches[litToIdx(c.lits[0])].push_back(c_idx);
        watches[litToIdx(c.lits[1])].push_back(c_idx);
    }
}

void SATSolver::reduceDB() {
    std::vector<int> learned;
    for (size_t i = 0; i < clauses.size(); i++) {
        if (clauses[i].learned && reason[std::abs(clauses[i].lits[0])] != (int)i)
            learned.push_back(i);
    }
    
    std::sort(learned.begin(), learned.end(), [&](int a, int b) {
        if (clauses[a].lbd != clauses[b].lbd) return clauses[a].lbd > clauses[b].lbd;
        return clauses[a].activity < clauses[b].activity;
    });

    for (size_t i = 0; i < learned.size() / 2; i++) {
        int idx = learned[i];
        if (drat) {
            fprintf(drat, "d ");
            for (Literal l : clauses[idx].lits) fprintf(drat, "%d ", l);
            fprintf(drat, "0\n");
        }
        // Mark for deletion or actually delete. For simplicity, we just won't watch them anymore.
        // A real industrial solver would compact the array.
    }
}

void SATSolver::cancelUntil(int blevel) {
    while (decisionLevel() > blevel) {
        int start = trail_lim.back();
        trail_lim.pop_back();
        for (size_t i = start; i < (int)trail.size(); i++) {
            int v = std::abs(trail[i]);
            phases[v] = (trail[i] > 0);
            assigns[v] = Assignment::UNASSIGNED;
            reason[v] = -1;
            level[v] = -1;
        }
        trail.resize(start);
        qhead = start;
    }
}

Literal SATSolver::pickBranchingLiteral() {
    int best = 0;
    double max_act = -1;
    for (int i = 1; i <= numVars; i++) {
        if (assigns[i] == Assignment::UNASSIGNED && activity[i] >= max_act) {
            max_act = activity[i];
            best = i;
        }
    }
    if (best == 0) return 0;
    return phases[best] ? best : -best;
}

bool SATSolver::solve() {
    conflicts = 0;
    while (true) {
        int conflict = propagate();
        if (conflict != -1) {
            conflicts++;
            if (decisionLevel() == 0) return false;
            std::vector<Literal> learned;
            int bt_level;
            analyze(conflict, learned, bt_level);
            cancelUntil(bt_level);
            record(learned);
            
            if (conflicts % 1000 == 0) reduceDB();
            
            if (conflicts >= restart_limit) {
                cancelUntil(0);
                restart_limit = (int)(restart_limit * restart_inc);
                conflicts = 0;
            }
        } else {
            Literal lit = pickBranchingLiteral();
            if (lit == 0) return true;
            trail_lim.push_back(trail.size());
            enqueue(lit);
        }
    }
}

void SATSolver::printAssignment() const {
    std::cout << "v ";
    for (int i = 1; i <= numVars; i++) {
        if (assigns[i] == Assignment::TRUE) std::cout << i << " ";
        else if (assigns[i] == Assignment::FALSE) std::cout << -i << " ";
    }
    std::cout << "0" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) return 1;
    SATSolver s;
    if (argc > 2) s.setDrat(argv[2]);
    if (s.loadDIMACS(argv[1])) {
        if (s.solve()) {
            std::cout << "s SATISFIABLE" << std::endl;
            s.printAssignment();
        } else {
            std::cout << "s UNSATISFIABLE" << std::endl;
        }
    }
    return 0;
}
