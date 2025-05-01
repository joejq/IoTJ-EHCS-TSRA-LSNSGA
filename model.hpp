/**
 * author: Qiangq Jiang
 * date: 2024.12.06
 * 
 * Main objective function
**/

#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include <cstring>
#include <cmath>
#include <list>
#include <queue>

#include "constant.h"
#include "iofile.hpp"

using namespace std;

/** Essential memory **/
// initial info from loading benchmark (constant data during one complete execution)
list<int> pred[MAX_N]; // precedent tasks
list<int> succ[MAX_N]; // subsequent tasks
int  indegree[MAX_N]; // the number of in-degree for each task
int  top_task[MAX_N]; // task sequence via topological sorting
float graph[MAX_N][MAX_N]; // edge weight between two tasks (namely, the data to be transferred between two tasks)
int task_cc[MAX_N][MAX_M]; // the number of clock cycles that current task need when executed on a specific core
int task_level[MAX_N]; // task level from DAG

// for executing objective function
float C_task[MAX_N];
float C_core[MAX_M];
float T_acitve[MAX_M];
float T_commun[MAX_M][MAX_M];
int Has_commun[MAX_N][MAX_N];

/** Global generational data **/
// benchamark scale: N-the number of tasks M-the number of cores H-the number of DVFS levels
int N, M, H;

// must load benchmark
bool bench_loaded = false;

// Topological sort
void topo_sort(int * seq, int *tl, int n) {
    queue<int> q;
    // get root/extrance task
    for(int i=0; i<n; ++i) {
        if(indegree[i] == 0) {
            q.push(i);
            tl[i] = 0;
            break;
        }
    }
    // sort other tasks
    int cnt = 0;
    while(!q.empty()) {
        int v = q.front();
        q.pop();
        seq[cnt] = v;
        ++ cnt;
        // get all post element from current vertex
        list<int>::iterator iter = succ[v].begin();
        for( ; iter!=succ[v].end(); ++iter) {
            tl[*iter] = tl[v] + 1;
            if(!(--indegree[*iter]))
                q.push(*iter); // enqueue if the indegree of current element equals 0 
        }
    }
    if(cnt < n) {
        cout << "[ERROR] Current DAG has cycles.\n";
        exit(-1);
    }
}

/*** Loading benchmark from local files ***/
// Loading benchmark inorder to build problem model
void load_benchmark(const char * path, int m, int h) {
    // if(bench_loaded) return;
    if(m > MAX_M) {
        cout << "[WARNING] The number of cores exceeds MAX value, and it will be forcibly set as MAX value.\n";
        m = MAX_M;
    }
    if(h > MAX_H) {
        cout << "[WARNING] The number of DVFS exceeds MAX value, and it will be forcibly set as MAX value.\n";
        h = MAX_H;
    }
    // set benchmark scale
    M = m;
    H = h;
    // init graph and adjacent list
    for(int i=0; i<MAX_N; ++i)
        for(int j=0; j<MAX_N; ++j)
            graph[i][j] = -1.0;
    // load test benchmark
    bench_loaded = (read_benchmark_stg(path, N, graph, pred, succ, indegree) &&
        read_stg_extra(path, m, graph, task_cc));
    if(!bench_loaded)
        exit(-1);
    topo_sort(top_task, task_level, N); // topological sort
}

/***  Objective functions  ***/
// Prepare data
void init_data(int n, int m) {
    memset(Has_commun, 0, sizeof(Has_commun)); // Has_commun
    for(int i=0; i<n; ++i) C_task[i] = -1.0; // C_task
    for(int j=0; j<m; ++j) {
        C_core[j] = 0.0;   // C_core
        T_acitve[j] = 0.0; // T_acitve
        for(int k=0; k<m; ++k)
            T_commun[j][k] = 0.0; // T_commun
    }
}

// Compute makespan
float compute_makespan(const int *task_list, const int *task_core, const int *core_dvfs, int n, int m) {
    if(!bench_loaded) {
        cout << "[ERROR] Can not compute makespan while benchmark data is not loaded.\n";
        exit(-1);
        // return 0.0;
    }
    ////////////
    int i, cnt = 0;
    int t_core[MAX_N];
    int t_dvfs[MAX_N];
    int *t_mark = new int[n];
    float Cmax = 0.0;
    
    init_data(n, m);
    memset(t_mark, 0, sizeof(int)*n); // init mark array
    // mapping task-core and task-DVFS level
    for(i=0; i<n; ++i) {
        t_core[task_list[i]] = task_core[i];
        t_dvfs[task_list[i]] = core_dvfs[i];
    }
    // calculate start/finish time according to DAG related benchmark
    while(cnt < n) {
        for(i=0; i<n; ++i) {
            if(t_mark[i]) continue;
            bool flag = true;
            float comp_cost;
            int task_x = task_list[i];
            int core_x = t_core[task_x];
            int dvfs_x = t_dvfs[task_x];
            if(pred[task_x].empty()) { // first or entrance task
                comp_cost = (float)task_cc[task_x][core_x] / core_freq[core_x][dvfs_x];
                C_task[task_x] = C_core[core_x] + comp_cost;
                // T_acitve[core_x] += comp_cost;
            } else { // other tasks
                // get the max finish time from all pred-tasks
                int pre_x;
                float trans_cost, pre_finish = 0.0;
                list<int>::iterator it = pred[task_x].begin();
                for( ; it != pred[task_x].end(); ++it) { // all tasks in preorder
                    pre_x = *it;
                    if(C_task[pre_x] < 0) { // pred/post relation fault tolerance ability
                        flag = false;
                        continue;
                    }
                    trans_cost = 0.0;
                    if(t_core[pre_x] != core_x) {
                        trans_cost = graph[pre_x][task_x]/(float)band[t_core[pre_x]][core_x];
                        if(!Has_commun[pre_x][task_x]) {
                            T_commun[t_core[pre_x]][core_x] += trans_cost;
                            // T_commun[t_core[pre_x]][core_x] += graph[pre_x][task_x];
                            Has_commun[pre_x][task_x] = 1;
                        } 
                        trans_cost += startup[core_x];
                    }
                    pre_finish = max(pre_finish, C_task[pre_x] + trans_cost);
                }
                if(flag) {
                    comp_cost = (float)task_cc[task_x][core_x] / core_freq[core_x][dvfs_x];
                    C_task[task_x] = max(pre_finish, C_core[core_x]) + comp_cost;
                }
                
            }
            // successfully calculate finish time 
            if(flag) {
                T_acitve[core_x] += comp_cost;
                C_core[core_x] = C_task[task_x];
                Cmax = max(Cmax, C_task[task_x]);
                t_mark[i] = 1;
                ++ cnt;
            } 
        }
    }
    // release memory
    delete [] t_mark;

    return Cmax;
}

// Compute active energy
float compute_active_energy(const int *task_list, const int *task_core, const int *core_dvfs, int n) {
    if(!bench_loaded) {
        cout << "[ERROR] Can not compute active energy while benchmark data is not loaded.\n";
        exit(-1);
    }
    //////////////
    int i;
    float active_energy = 0.0; // KJ
    for(i=0; i<n; ++i) {
        int task_x = task_list[i];
        int core_x = task_core[i];
        int dvfs_x = core_dvfs[i];
        active_energy += circuit_capacity * (core_vol[core_x][dvfs_x] * core_vol[core_x][dvfs_x]) * task_cc[task_x][core_x];
    }
    // KJ->J
    active_energy *= 1e+3;
    return active_energy;
}

// Compute static/idle energy
float compute_static_energy(float makespan, int m) {
    if(!bench_loaded) {
        cout << "[ERROR] Can not compute static energy while benchmark data is not loaded.\n";
        exit(-1);
    }
    /////////////
    float static_energy = 0.0; // Ws = J
    float time;
    for(int j=0; j<m; ++j)
        static_energy += (makespan-T_acitve[j]) * core_idle[j];
    return static_energy; 
}

// Compute communication energy
float compute_commun_energy(int m) {
    if(!bench_loaded) {
        cout << "[ERROR] Can not compute communication energy while benchmark data is not loaded.\n";
        exit(-1);
    }
    ///////////
    float commun_energy = 0.0; // GJ (=2^30 J ~= 1e+9 J)
    for(int j=0; j<m; ++j)
        for(int k=0; k<m; ++k)
            commun_energy += T_commun[j][k] * (manhattan[j][k] * E_link + E_router);
            // cout << j << "-" << k << " t_c=" << T_commun[j][k] << " E_com=" << T_commun[j][k] * (manhattan[j][k] * E_link + E_router) << endl;
    // GJ -> J
    // commun_energy *= 1e+9;
    return commun_energy;
}

// Compute total energy consumption
float compute_energy(float makespan, const int *task_list, const int *task_core, const int *core_dvfs, int n, int m) {
    if(!bench_loaded) {
        cout << "[ERROR] Can not compute total energy while benchmark data is not loaded.\n";
        exit(-1);
    }
    ////////////
    float e_active = compute_active_energy(task_list, task_core, core_dvfs, n);
    float e_static = compute_static_energy(makespan, m);
    float e_commun = compute_commun_energy(m);

    return e_active + e_static + e_commun;
}

#endif
