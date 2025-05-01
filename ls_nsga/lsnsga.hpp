/**
 * author: Qiangq Jiang
 * date: 2025.01.02
 * 
 * NSGA-II
 **/

#ifndef NSGA_H
#define NSGA_H

#include "core.hpp"

/***********core data***********/
#define MAX_ITER 500 
#define POP_SIZE 50   // !!!! POP_SIZE must be n * 10
#define MUTATION_RATE 0.2
#define CROSSX_RATE 0.8

vector<Chromosome> Pt;
vector<Chromosome> Qt;
vector<Chromosome> Rt;
map<int, vector<Chromosome> > F;

vector<Chromosome> pareto_set;
vector<Chromosome> new_slts;
int task_n[MAX_N][MAX_N], core_n[COMBN][MAX_N], dvfs_n[COMBN][MAX_N];

/***********core computation methods***********/
void run(int num_runs, const char * output_path);

void run(int num_runs, const char * output_path) {
    if(!bench_loaded) {
        cout << "[ERROR] The benchmark data is not loaded.\n";
        exit(-1);
    }
    
    clock_t start, finish; // time counter
	double duration = 0.0;

    for(int i=1; i<=num_runs; ++i) {
        cout << "Runs: " << i << endl;
        start = clock();
        /***/
        main_process();
        /***/
        finish = clock();
        duration = (double)(finish - start) / CLOCKS_PER_SEC;
        // sort the final results by makespan asc
        extract_pareto_set(pareto_set);
        sort(pareto_set.begin(), pareto_set.end(), compare_makespan);
        write_file(output_path, pareto_set, duration);
        cout << "latency: " << duration << endl;
        cout << endl;
    }
}

/*rtg_loading benchmark*/
void rtg_loading(int m=INPUT_M, int h=INPUT_H, int no_rand=INPUT_NO) {
    if(m <= 0 || h <= 0) {
        cout << "[ERROR] Invalid inputs.\n";
        exit(-1);
    }
    if(m > MAX_M || h > MAX_H) {
        m = min(m, MAX_M);
        h = min(h, MAX_H);
        cout << "[WARNING] The parameter 'm' or 'h' overflow the maximum range, and it will be reset as the maximum value.\n";
    }
    // seed for random number generator
    srand((unsigned)time(NULL)); 
    // load benchmark from local files
    char bench_name[128];
    if(no_rand < 10) sprintf(bench_name, "rand000%d.stg", no_rand);
    else if(no_rand < 100) sprintf(bench_name, "rand00%d.stg", no_rand);
    else sprintf(bench_name, "rand0%d.stg", no_rand);
    load_benchmark(bench_name, m, h);
    // print title
    cout << "----------------------------------------------------------------------" << endl;
    cout << "                            Hello, LS-NSGA                            " << endl;
    cout << "----------------------------------------------------------------------" << endl;
    cout << "[SCALE] " << NUM_TASK << " * " << M << " * " << H << "\n\n";
}
/*rtg_loading benchmark for real application*/
void pa_loading(int m=INPUT_M, int h=INPUT_H, int ra_no=INPUT_NO) {
    if(m <= 0 || h <= 0) {
        cout << "[ERROR] Invalid inputs.\n";
        exit(-1);
    }
    if(m > MAX_M || h > MAX_H) {
        m = min(m, MAX_M);
        h = min(h, MAX_H);
        cout << "[WARNING] The parameter 'm' or 'h' overflow the maximum range, and it will be reset as the maximum value.\n";
    }
    // seed for random number generator
    srand((unsigned)time(NULL)); 
    // load benchmark from local files
    char bench_name[128];
    sprintf(bench_name, "%s.stg", ra_cases[ra_no].c_str());
    load_benchmark(bench_name, m, h);
    // print title
    cout << "----------------------------------------------------------------------" << endl;
    cout << "                            Hello, LS-NSGA                            " << endl;
    cout << "----------------------------------------------------------------------" << endl;
    cout << "[SCALE] " << NUM_TASK << " * " << M << " * " << H << "\n\n";
}

/*generate initial population*/
void initialize() {
    Pt.clear();
    Qt.clear();
    Rt.clear();
    F.clear();
    pareto_set.clear();
    for(int i=0; i<POP_SIZE; ++i) {
        Chromosome chm(N);
        rand_sequence(chm.task, N);
        for(int j=0; j<N; ++j) {
            chm.core[j] = get_rand(0, M);
            chm.dvfs[j] = get_rand(0, H);
        }
        // printc(chm);
        update_fitness(chm);
        Pt.push_back(chm);
    }
}
/*generate new generation by parent*/
void make_new_generation() {
    Qt.clear();
    Rt.clear();
    int r, i;
    int a1[POP_SIZE];
    int a2[POP_SIZE];
    Chromosome parent1, parent2;
    // get random sequence
    rand_sequence(a1, POP_SIZE);
    rand_sequence(a2, POP_SIZE);
    // crossover
    for(i=0; (i+4)<=POP_SIZE; i+=4) {
        // print(Pt[a1[i]]);
        parent1 = binary_tournament(Pt[a1[i]], Pt[a1[i+1]]);
        parent2 = binary_tournament(Pt[a1[i+2]], Pt[a1[i+3]]);
        if(get_frand(0, 1) <= CROSSX_RATE) multi_crossover(parent1, parent2);
        Qt.push_back(parent1);
        Qt.push_back(parent2);
        parent1 = binary_tournament(Pt[a2[i]], Pt[a2[i+1]]);
        parent2 = binary_tournament(Pt[a2[i+2]], Pt[a2[i+3]]);
        if(get_frand(0, 1) <= CROSSX_RATE) multi_crossover(parent1, parent2);
        Qt.push_back(parent1);
        Qt.push_back(parent2);        
    }
    if(POP_SIZE % 4 != 0) {
        parent1 = binary_tournament(Pt[a1[POP_SIZE-2]], Pt[a1[POP_SIZE-1]]);
        parent2 = binary_tournament(Pt[a2[POP_SIZE-2]], Pt[a2[POP_SIZE-1]]);
        if(get_frand(0, 1) <= 0.8) multi_crossover(parent1, parent2);
        Qt.push_back(parent1);
        Qt.push_back(parent2);
    }
    // mutation
    for(i=0; i<POP_SIZE; ++i) {
        if(get_frand(0, 1) <= MUTATION_RATE) 
            multi_mutation(Qt[i]);
        update_fitness(Qt[i]);
    }
    
    Rt.assign(Pt.begin(), Pt.end());
    Rt.insert(Rt.end(), Qt.begin(), Qt.end());
}
/*fast nondominated sort*/
void fast_nondominated_sort(vector<Chromosome> & swarm) {
    F.clear();
    bool b;
    int rank = 0, count = 0;
    int size = swarm.size();
    short *rmv = new short[size]{0};
    int i, j, k;
    while(count < size) {
        vector<Chromosome> res;
        short *mark = new short[size]{0};
        for(i=0; i<size; ++i) {
            if(rmv[i]) continue;
            b = true;
            for(j=0; j<size; ++j) {
                if(i == j || rmv[j]) continue;
                if(dominate(swarm[j], swarm[i])) {
                    b = false;
                    break;
                }
            }
            if(b) {
                mark[i] = 1;
                swarm[i].rank = rank;
                F[rank].push_back(swarm[i]);
                count ++;
            }
        }
        for(k=0; k<size; ++k) if(mark[k]) rmv[k] = 1;
        delete [] mark;
        rank ++;
    }
    delete [] rmv;
}
/*desc sorting by crowding distance*/
void crowding_distance_assignment(vector<Chromosome> & swarm) {
    if(swarm.size() <= 0) return;
    int size = swarm.size();
    int i;
    for(i=0; i<size; ++i) swarm[i].crowd_distance = 0;
    // makespan distance calculation
    sort(swarm.begin(), swarm.end(), compare_makespan);
    
    swarm[0].crowd_distance = swarm[size-1].crowd_distance = FLT_MAX;
    for(i=1; i<size-1; ++i) 
        swarm[i].crowd_distance += 
            swarm[i+1].fitness.makespan - swarm[i-1].fitness.makespan;
    // energy distance calculation
    sort(swarm.begin(), swarm.end(), compare_energy);
    swarm[0].crowd_distance = swarm[size-1].crowd_distance = FLT_MAX;
    for(i=1; i<size-1; ++i) 
        swarm[i].crowd_distance += 
            swarm[i+1].fitness.energy - swarm[i-1].fitness.energy;
    // desc sort
    sort(swarm.begin(), swarm.end(), compare_cd);
}

/*Enumerate possible construction*/
void construct(const int * task, int n, int num) {
    new_slts.clear();
    int i, k;
    for(i=0; i<num; ++i) {
        Chromosome s(n, task, core_n[i], dvfs_n[i]);
        if(n == N) 
            update_fitness(s);
        else 
            update_fitness_partial(s, n);
        new_slts.push_back(s);
    }
    return;
}
void construct(const int * core, const int * dvfs, int n, int num) {
    new_slts.clear();
    int i, k;
    for(i=0; i<num; ++i) {
        Chromosome s(n, task_n[i], core, dvfs);
        if(n == N) 
            update_fitness(s);
        else 
            update_fitness_partial(s, n);
        new_slts.push_back(s);
    }
    return;
}
/*Key metrics for selecting the best Chromosome from all solutions*/
void best_selection(const Chromosome & slt, Chromosome & best_slt) {
    if(new_slts.empty()) return;
    // extract_pareto_set(new_slts);
    extract_batch_pareto(new_slts); // new extracting function
    if(new_slts.front().len == N) {  /// complete job squence
        pareto_set.insert(pareto_set.end(), new_slts.begin(), new_slts.end());
        vector<Chromosome> dom_slt; // store the solutions that can dominate the current Chromosome
        // if has dominated Chromosome
        for(int i=0; i<new_slts.size(); ++i)
            if(dominate(new_slts[i], slt))
                dom_slt.push_back(new_slts[i]);
        if(dom_slt.size() > 0) {
            int r = get_rand(0, dom_slt.size());
            chmcopy(best_slt, dom_slt.at(r));
            return;
        }
    }
    // if has not dominated Chromosome
    /// partial job squence in construct process of IG
    int best_i;
    float p = get_frand(0, 1);
    if(p <= 0.5) { // p <= 0.5 random
        best_i =  rand() % (new_slts.size());
    } else { // p > 0.5 select the minimal makespan or energy Chromosome
        best_i = 0;
        p = get_frand(0, 1);
        for(int i=1; i<new_slts.size(); ++i) {
            if(p <= 0.5) {
                if(new_slts[i].fitness.makespan < new_slts[best_i].fitness.makespan) 
                    best_i = i;
            } else {
                if(new_slts[i].fitness.energy < new_slts[best_i].fitness.energy) 
                    best_i = i;
            }
        }
    }
    chmcopy(best_slt, new_slts.at(best_i));
    return;
}
/*Local search strategy*/
Chromosome ls_strategy(const Chromosome & slt) {
    int i, j, k, n = slt.len;
    Chromosome best_slt(slt);
    for(i=0; i<n; ++i) { 
        int cnt = 0;
        for(j=0; j<M; ++j) {
            for(k=0; k<H; ++k) {
                memcpy(core_n[cnt], best_slt.core, sizeof(int) * n);
                memcpy(dvfs_n[cnt], best_slt.dvfs, sizeof(int) * n);
                core_n[cnt][i] = j;
                dvfs_n[cnt][i] = k;
                ++ cnt;
            }
        }
        construct(slt.task, slt.len, cnt);
        best_selection(slt, best_slt);
    }
    return best_slt;
}

/*core process of NSGA2*/
void main_process() {
    initialize();
    int t = 0;
    while(t < MAX_ITER) {
        // update pareo set
        pareto_set.insert(pareto_set.end(), Pt.begin(), Pt.end());
        extract_batch_pareto(pareto_set);// new extracting function
        // GA operation
        make_new_generation();
        fast_nondominated_sort(Rt);
        Pt.clear();
        int i = 0;
        while(Pt.size() + F[i].size() < POP_SIZE) {
            crowding_distance_assignment(F[i]);
            Pt.insert(Pt.end(), F[i].begin(), F[i].end());
            ++ i;
        }
        crowding_distance_assignment(F[i]);
        Pt.insert(Pt.end(), F[i].begin(), F[i].begin()+(POP_SIZE-Pt.size()));
        /******LS operation*******/
        if(get_frand(0, 1) < 0.5) {
            int r = get_rand(0, F[0].size());
            ls_strategy(F[0][r]);
        }
        ++ t;
    }
}

#endif