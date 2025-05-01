/**
 * author: Qiangq Jiang
 * date: 2025.01.02
 * 
 * Core data structure for NSGA-II
**/

#ifndef CHM_H
#define CHM_H

#include <cstring>
#include <iostream>
#include <cstdlib>

#include "../config.h"

using namespace std;

struct Fitness {
    float makespan;
    float energy;
    Fitness(): makespan(0), energy(0) {}
    Fitness(float ms, float e): makespan(ms), energy(e) {}
};

struct Chromosome {
    int task[MAX_N];
    int core[MAX_N];
    int dvfs[MAX_N];
    int len;
    int rank;
    float crowd_distance;
    Fitness fitness;
    Chromosome();
    Chromosome(int n);
    Chromosome(int n, const int * t, const int * c, const int * d, int cd=0, int rk=-1);
    Chromosome(const Chromosome & chm);
};
// constructing functions
Chromosome::Chromosome(): rank(-1), crowd_distance(0) {
    len = MAX_N;
}
Chromosome::Chromosome(int n) {
    len = n;
}
Chromosome::Chromosome(int n, const int * t, const int * c, const int * d, int cd, int rk) {
    len = n;
    rank = rk;
    crowd_distance = cd;
    memcpy(task, t, sizeof(int) * len);
    memcpy(core, c, sizeof(int) * len);
    memcpy(dvfs, d, sizeof(int) * len);
}
Chromosome::Chromosome(const Chromosome & chm) {
    len = chm.len;
    rank = chm.rank;
    crowd_distance = chm.crowd_distance;
    memcpy(task, chm.task, sizeof(int) * chm.len);
    memcpy(core, chm.core, sizeof(int) * chm.len);
    memcpy(dvfs, chm.dvfs, sizeof(int) * chm.len);
    fitness.energy = chm.fitness.energy;
    fitness.makespan = chm.fitness.makespan;
}
//crowd_distance desc
bool compare_cd(const Chromosome &x, const Chromosome &y) {
	return x.crowd_distance > y.crowd_distance;
}
// makespan asc
bool compare_makespan(const Chromosome &x, const Chromosome &y) {
	return x.fitness.makespan < y.fitness.makespan;
}
// energy asc
bool compare_energy(const Chromosome &x, const Chromosome &y) {
	return x.fitness.energy < y.fitness.energy;
}
// copy function
void chmcopy(Chromosome & des_chm, const Chromosome & src_chm) {
    des_chm.len = src_chm.len;
    des_chm.rank = src_chm.rank;
    des_chm.crowd_distance = src_chm.crowd_distance;
    memcpy(des_chm.task, src_chm.task, sizeof(int) * des_chm.len);
    memcpy(des_chm.core, src_chm.core, sizeof(int) * des_chm.len);
    memcpy(des_chm.dvfs, src_chm.dvfs, sizeof(int) * des_chm.len);
    des_chm.fitness.energy = src_chm.fitness.energy;
    des_chm.fitness.makespan = src_chm.fitness.makespan;
}
/*dedicated print utils*/
void printc(const Chromosome &chm) {
    cout << "task:[ ";
    for(int i=0; i<chm.len; ++i) cout << chm.task[i] << " ";
    cout << "] core:[ ";
    for(int i=0; i<chm.len; ++i) cout << chm.core[i] << " ";
    cout << "] dvfs:[ ";
    for(int i=0; i<chm.len; ++i) cout << chm.dvfs[i] << " ";
    cout << "] makespan = " << chm.fitness.makespan <<" energy = " << chm.fitness.energy << endl;
}


#endif