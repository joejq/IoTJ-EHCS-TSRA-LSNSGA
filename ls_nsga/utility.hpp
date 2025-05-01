/**
 * author: Qiangq Jiang
 * date: 2025.01.02
 * 
 * Generic functions for NSGA-II and MOEA/D
**/
#ifndef UTILITY_H
#define UTILITY_H

#include <cmath>
#include <cstring>

using namespace std;

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

/*Generate random integer number of [lb, ub)*/
int get_rand(int lb, int ub) {
    return  rand() % (ub - lb ) + lb;
}
/*Generate random float number of [lb, ub)*/
float get_frand(float lb, float ub) {
    float randx = (float)rand() / RAND_MAX;
    return lb + (ub - lb) * randx;
}

/*Swap two elements from different position in the array*/
void swap(int * arr, int px, int py) {
    int temp = arr[px];
    arr[px] = arr[py];
    arr[py] = temp;
}

/*crossover strategy (PMX)*/
void crossover(int *seqx, int * seqy, int n, int left, int right) {
    int i;
    int * mark1 = new int[n];
    int * mark2 = new int[n];
    int * nmark1 = new int[n];
    int * nmark2 = new int[n];

    for(i=0; i<n; ++i) mark1[i] = mark2[i] = -1;
    for(i=left; i<=right; ++i) {
        int temp = seqx[i];
        seqx[i] = seqy[i];
        seqy[i] = temp;
        mark1[seqx[i]] = seqy[i];
        mark2[seqy[i]] = seqx[i];
    }
    // mend
    memcpy(nmark1, mark1, n * sizeof(int));
    memcpy(nmark2, mark2, n * sizeof(int));
    for(i=left; i<=right; ++i) {
        int j, v;
        j = seqx[i];
        v = mark1[j];
        while(v != j && mark1[v] != -1) {
            nmark1[j] = mark1[v];
            v = nmark1[j];
        }
        j = seqy[i];
        v = mark2[j];
        while(v != j && mark2[v] != -1) {
            nmark2[j] = mark2[v];
            v = nmark2[j];
        }
    }
    for(i=0; i<n; ++i) {
        if(i >= left && i <= right) continue;
        if(nmark1[seqx[i]] != -1) seqx[i] = nmark1[seqx[i]];
        if(nmark2[seqy[i]] != -1) seqy[i] = nmark2[seqy[i]];
    }

    delete [] mark1;
    delete [] mark2;
    delete [] nmark1;
    delete [] nmark2;
}
/*Corssover strategy DE*/
void DE_crossover(int n, const int *src, const int *p1, const int *p2, int *des) {
    int low = 0, up = n - 1;
    vector<int> vec;
    short * mark = new short[n] {0};
    int res, interval = up - low;
    // DE distory
    for(int i=0; i<n; ++i) {
        res = src[i] - (abs(p1[i]-p2[i]));
        if(res < 0) // repair
            res = low + (res + interval) % (interval);
        if(mark[res])
            vec.push_back(i); // 'res' valued task has existed
        else
            mark[res] = 1;
        des[i] = res;
    }
    // build new sequence
    int k = 0;
    for(int i=0; i<n; ++i) {
        if(!mark[src[i]]) {
            des[vec.at(k)] = src[i];
            ++ k;
        }
    }
    delete [] mark;
    // return Chromosome(arr);
}
/*create a random sequence whose range is [0, n-1]*/
void rand_sequence(int * seq, int n) {
    for(int i=0; i<n; ++i)
        seq[i] = i;
    for(int i=0; i<n; ++i) {
        int r = get_rand(0, n);
        swap(seq, r, i);
    }
}
/*mutation strategy*/
void mutation(int *seq, int n) {
    int r1 = get_rand(0, n);
    int r2 = r1;
    while(r2 == r1) r2 = get_rand(0, n);
    swap(seq, r1, r2);
}
/*Insert task from 'src_pos' position into 'des_pos' position*/
void inserts(int * seq, int n, int src_pos, int des_pos) {
    int task_x = seq[src_pos];
    int * tseq = new int[n];
    memcpy(tseq, seq, (src_pos) * sizeof(int));
    memcpy(tseq+src_pos, seq+src_pos+1, (n-src_pos-1) * sizeof(int));
    memcpy(seq, tseq, des_pos * sizeof(int));
    seq[des_pos] = task_x;
    memcpy(seq+des_pos+1, tseq+des_pos, (n-des_pos-1) * sizeof(int));
        
    delete [] tseq;
}

/*Search*/
int search_element(const int * arr, int size, int target)
{
	int low = 0, high = size;
    for(int i=0; i<size; ++i) {
        if(arr[i] == target)
            return i;
    }
	return -1;
}

#endif