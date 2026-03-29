#include "../include/sat_solver.h"
#include "../include/low_level.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cmath>

/* 
 * The main solver. We're using CDCL now because DPLL 
 * was just too damn slow on the bigger PHP instances.
 */

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
            
            // Allocation hell
            assigns.assign(numVars + 1, Assignment::UNASSIGNED);
            level.assign(numVars + 1, -1);
            reason.assign(numVars + 1, -1);
            activity.assign(numVars + 1, 0.0);
            watches.assign((numVars + 1) * 2, std::vector<int>());
            seen.assign(numVars + 1, false);
            
            restart_limit = 100;
            restart_inc = 1.1;
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
                if (!enqueue(c.lits[0])) return false; // Immediate contradiction
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

// 2-Watched Literals. This is where the magic (and the speed) happens.
int SATSolver::propagate() {
    while (qhead < (int)trail.size()) {
        Literal p = trail[qhead++];
        int p_idx = litToIdx(negLit(p));
        auto& ws = watches[p_idx];
        
        for (size_t i = 0; i < ws.size(); ) {
            int c_idx = ws[i];
            Clause& c = clauses[c_idx];
            
            if (c.lits[0] == negLit(p)) std::swap(c.lits[0], c.lits[1]);
            
            // Already happy?
            if (is_literal_satisfied(c.lits[0], (int)assigns[std::abs(c.lits[0])])) {
                i++; continue;
            }
            
            bool found = false;
            for (size_t j = 2; j < c.lits.size(); j++) {
                if (!is_literal_falsified(c.lits[j], (int)assigns[std::abs(c.lits[j])])) {
                    std::swap(c.lits[1], c.lits[j]);
                    watches[litToIdx(c.lits[1])].push_back(c_idx);
                    ws[i] = ws.back();
                    ws.pop_back();
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                if (!enqueue(c.lits[0], c_idx)) return c_idx; // Conflict!
                i++;
            }
        }
    }
    return -1;
}

// 1-UIP Conflict Analysis. It's ugly, don't touch it.
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
    
    var_inc *= 1.05; // Activity decay
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
    clauses.push_back(c);
    enqueue(c.lits[0], c_idx);
    if (c.lits.size() > 1) {
        watches[litToIdx(c.lits[0])].push_back(c_idx);
        watches[litToIdx(c.lits[1])].push_back(c_idx);
    }
}

void SATSolver::cancelUntil(int blevel) {
    while (decisionLevel() > blevel) {
        int start = trail_lim.back();
        trail_lim.pop_back();
        for (size_t i = start; i < trail.size(); i++) {
            int v = std::abs(trail[i]);
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
    return (best == 0) ? 0 : best;
}

bool SATSolver::solve() {
    int conflicts = 0;
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
    if (argc < 2) {
        std::cerr << "Need a file, bro." << std::endl;
        return 1;
    }
    SATSolver s;
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
