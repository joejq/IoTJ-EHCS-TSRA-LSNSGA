/**
 * author: Qiangq Jiang
 * date: 2025.01.02
 * 
 * Specific functions for NSGA-II
**/
#ifndef CORE_H
#define CORE_H

#include <memory.h>
#include <time.h>
#include <float.h>
#include <algorithm>
#include <vector>
#include <map>
#include <stack>

#include "../model.hpp"
#include "../iofile.hpp"
#include "utility.hpp"
#include "chromosome.hpp"

using namespace std;

// // Windows OS
// #ifdef _WIN32
// // #define PRE_OUT_PATH "D:\\NutCloud\\FSPSeries\\HMTS\\code_experiments\\program\\comparison\\multi_objective\\nsga\\out\\"
// #define PRE_OUT_PATH "E:\\NutCloud\\FSPSeries\\HMTS\\code_experiments\\program\\comparison\\multi_objective\\nsga\\out\\"
// #endif

/*Compute the fitness for solution with partial tasks*/
void update_fitness_partial(Chromosome & slt, int n) {
    slt.fitness.makespan = compute_makespan(slt.task, slt.core, slt.dvfs, n, M);
    slt.fitness.energy = compute_energy(slt.fitness.makespan, slt.task, slt.core, slt.dvfs, n, M);
}
/*Compute the fitness for current chromosome*/
void update_fitness(Chromosome & chm) {
    chm.fitness.makespan = compute_makespan(chm.task, chm.core, chm.dvfs, N, M);
    chm.fitness.energy = compute_energy(chm.fitness.makespan, chm.task, chm.core, chm.dvfs, N, M);
}
/*Determine if the two solutions are same*/
bool slt_same(const Chromosome & x, const Chromosome & y) {
    for(int i=0; i<x.len; ++i) {
        if(x.task[i] != y.task[i] 
        || x.core[i] != y.core[i]
        || x.dvfs[i] != y.dvfs[i])
            return false;
    }
    return true;
}
/*Determine if these two solutions have same objective function value*/
bool obj_same(const Chromosome & x, const Chromosome & y) {
    if(x.fitness.makespan == y.fitness.makespan &&
       x.fitness.energy == y.fitness.energy) return true;
    return false;
}
/*Determine if x dominate y, if so return true and else return false*/
bool dominate(const Chromosome & x, const Chromosome & y) {
    if(
        x.fitness.makespan <= y.fitness.makespan &&
        x.fitness.energy < y.fitness.energy ||
        x.fitness.makespan < y.fitness.makespan &&
        x.fitness.energy <= y.fitness.energy)
        return true;
    else return false;
}
/*Get pareto set from all solutions and store these ps into original set*/
void extract_pareto_set(vector<Chromosome> & swarm) {
    vector<Chromosome> res;
    bool b;
    int size = swarm.size();
    short *mark = new short[size] {0};
    for(int i=0; i<size; ++i) {
        b = true;
        for(int j=0; j<size; ++j) {
            if(i == j) continue;
            // if(slt_same(swarm[i], swarm[j])) {
            if(obj_same(swarm[i], swarm[j])) {
                mark[j] = 1;
                continue;
            }
            if(dominate(swarm[j], swarm[i])) {
                b = false;
                break;
            }
        }
        if(b && !mark[i]) res.push_back(swarm[i]);
    }
    delete [] mark;
    swarm.clear();
    swarm.assign(res.begin(), res.end());
}
/*Get pareto set from all solutions within a fixed batch*/
void extract_batch_pareto(vector<Chromosome> & swarm, int batch_size=BATCH_CAPACITY) {
    extract_pareto_set(swarm);
    // exceed the max capacity
    if(swarm.size() > batch_size) {
        // randomly select 'max capacity' pareto solutions
        vector<Chromosome> res;
        int seq[batch_size];
        rand_sequence(seq, batch_size);
        for(int i=0; i<batch_size; ++i)
            res.push_back(swarm.at(seq[i]));
        swarm.clear();
        swarm.assign(res.begin(), res.end());
    }
    return;
}

/*get better gene*/
Chromosome binary_tournament(const Chromosome & x, const Chromosome & y) {
    if(x.rank < y.rank) return x;
    else if (x.rank == y.rank) {
        if(x.crowd_distance >= y.crowd_distance) return x;
        else return y;
    } 
    else return y;
}
/*Multi-sequence crossover(PMX)*/
void multi_crossover(Chromosome & x, Chromosome & y) {
    int n = x.len;
    int r1 = get_rand(0, n);
    int r2 = r1;
    while(r2 == r1) r2 = get_rand(0, n);
    int left = min(r1, r2);
    int right = max(r1, r2);
    // task list
    crossover(x.task, y.task, n, left, right);
    // core and dvfs list
    for(int i=left; i<=right; ++i) {
        int temp = x.core[i];
        x.core[i] = y.core[i];
        y.core[i] = temp;
        temp = x.dvfs[i];
        x.dvfs[i] = y.dvfs[i];
        y.dvfs[i] = temp;
    }
}
/*Multi-sequence mutation*/
void multi_mutation(Chromosome & chm) {
    int n = chm.len;
    int r1 = get_rand(0, n);
    int r2 = r1;
    while(r2 == r1) r2 = get_rand(0, n);
    // task list
    swap(chm.task, r1, r2);
    // core and dvfs list
    chm.core[r1] = get_rand(0, M);
    chm.core[r2] = get_rand(0, M);
    chm.dvfs[r1] = get_rand(0, H);
    chm.dvfs[r2] = get_rand(0, H);
}
// Write slution set into local file
void write_file(const char * path, const vector<Chromosome> & swarm, double time=-1.0) {
    char fpath[128] = {0};
    path_fill(fpath, PRE_OUT_PATH, path);
    cout << "Start writing result to '" << fpath << "' ..." << endl; 
    ofstream outfile;   //out stream
    outfile.open(fpath, ios::app);   // ios::app - Append content at the end 
    if(!outfile.is_open ()) {
        cout << "error: open file failure" << endl;
        exit(-1);
    }
    // outfile << "=======================> pareto set as follows:" << endl;
    // read file and store into memory
    int i;
    for (auto e: swarm) {
        outfile << "task:[ ";
        for(i=0; i<e.len; ++i) outfile << e.task[i] << " ";
        outfile << "] core:[ ";
        for(i=0; i<e.len; ++i) outfile << e.core[i] << " ";
        outfile << "] dvfs:[ ";
        for(i=0; i<e.len; ++i) outfile << e.dvfs[i] << " ";
        outfile << "] makespan = " << e.fitness.makespan <<" energy = " << e.fitness.energy << endl;
    }
    // if have passed parameters-'latency' here
    if(time >= 0) outfile << "latency:" << time << endl;
    outfile << endl;
    outfile.close();
    cout << "Complete writing!" << endl;
}

#endif