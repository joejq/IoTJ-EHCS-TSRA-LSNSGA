/**
 * author: Qiangq Jiang
 * date: 2024.12.06
 * 
 * File access with write/read operations
**/
#ifndef IOFILE_H
#define IOFILE_H


#ifdef _WIN32
#include <io.h>
#elif __linux__
#include <sys/io.h>
#endif
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "config.h"

using namespace std;

// Merge prefix path
void path_fill(char * fpath, const char * pre, const char * path) {
    strcpy(fpath, pre);
    strcat(fpath, path);
    // cout << fpath << endl;
}

// Convert string into float value
float str2val(const string & str) {
    float res;
    stringstream ss;  
    ss << str;                  
    ss >> res;
    return res;
}

// Read benchmark from local file-matrix format
bool read_benchmark(const char * path, int &n, int &m, float (*graph)[MAX_N], int (*task_cc)[MAX_M]) {
    char fpath[128] = {0};
    path_fill(fpath, PRE_BENCH_PATH, path);
    cout << "Start loading benchmark by reading '" << fpath << "' ...\n";
    ifstream infile(fpath);
    if(!infile) {
        cout <<"error: no such file\n";
        return false;
        // exit(-1);
    }
    // read file and store into memory
    string line, digitstr;
    int i = 0, type = 0;
    while (getline(infile, line)) {
        if(line.size() <= 0) continue;
        if(line.substr(0, 1).compare("*") == 0) continue;
        if(line.substr(0, 1).compare("#") == 0) {
            i = 0;
            ++ type;
            continue;
        }
        int j = 0;
        istringstream iss(line);
        switch (type)
        {
        case 1: // DAG graph
            while(iss >> digitstr) {
                stringstream ss;
                ss << digitstr;                  
                ss >> graph[i][j];
                ++ j;
            }
            n = j;
            break;
        case 2: // task clock cycles
            while(iss >> digitstr) {
                stringstream ss;  
                ss << digitstr;                  
                ss >> task_cc[i][j];
                ++ j;
            }
            m = j;
            break;
        
        default:
            cout << "invalid input\n";
            break;
        }
        ++ i;
    }
    infile.close();
    cout << "Complete loading!\n";
    return true;
}

// Read benchmark from local file-link list format
bool read_benchmark(const char * path, int &n, int m, float (*graph)[MAX_N], int (*task_cc)[MAX_M], list<int> * pre, list<int> * adj, int * indegree) {
    char fpath[128] = {0};
    path_fill(fpath, PRE_BENCH_PATH, path);
    cout << "Start loading benchmark by reading '" << fpath << "' ...\n";
    ifstream infile(fpath);
    if(!infile) {
        cout <<"error: no such file\n";
        return false;
        // exit(-1);
    }
    // read file and store into memory
    string line, digitstr;
    int i = 0, type = 0;
    int u, v, w;
    bool first = true;
    while (getline(infile, line)) {
        if(line.size() <= 0) continue;
        if(line.substr(0, 1).compare("*") == 0) continue;
        if(line.substr(0, 1).compare("#") == 0) {
            i = 0;
            ++ type;
            continue;
        }
        int j = 0;
        istringstream iss(line);
        switch (type)
        {
        case 1: // DAG graph
            if(first) {
                iss >> n;
                memset(indegree, 0, sizeof(int) * n);
                first = false;
            } else {
                iss >> u;
                iss >> v;
                iss >> w;
                graph[u][v] = w;
                adj[u].push_back(v);
                pre[v].push_back(u);
                ++ indegree[v];
            }
            break;
        case 2: // task clock cycles
            while(j < m) {
                iss >> task_cc[i][j];
                ++ j;
            }
            // m = j;
            break;
        
        default:
            cout << "invalid input\n";
            break;
        }
        ++ i;
    }
    infile.close();
    cout << "Complete loading!\n";
    return true;
}

// Read benchmark from STG
bool read_benchmark_stg(const char * path, int &n, float (*graph)[MAX_N], list<int> * pre, list<int> * adj, int * indegree) {
    char tpath[128];
    path_fill(tpath, PRE_STG_PATH, path);
    char fpath[512];
    sprintf(fpath, tpath, NUM_TASK);
    cout << "Start loading STG benchmark by reading '" << fpath << "' ...\n";
    ifstream infile(fpath);
    if(!infile) {
        cout <<"error: no such file\n";
        return false;
    }
    // read file and store into memory
    string line;
    int u, v, w; // current task id, precedent task id, execution cost of current task
    bool first = true;
    while (getline(infile, line)) {
        if(line.size() <= 0) continue;
        if(line.substr(0, 1).compare("#") == 0) break;
        istringstream iss(line);
        if(first) {
            iss >> n;
            n += 2; // first and last task are dummy tasks
            first = false;
        } else {
            iss >> u; // current task id
            iss >> w; // exe cost
            iss >> indegree[u]; // the number of indegree for task_u
            for(int j=0; j<indegree[u]; ++j) { // put all precedent tasks & subsequent tasks into list
                iss >> v;
                graph[v][u] = 1;
                adj[v].push_back(u);
                pre[u].push_back(v);
            }
        }
        
    }
    infile.close();
    cout << "Complete loading STG benchmark!\n";
    return true;
}

// Read extra data for STG
bool read_stg_extra(const char * path, int m, float (*graph)[MAX_N], int (*task_cc)[MAX_M]) {
    char tpath[128];
    path_fill(tpath, PRE_STG_EXTRA_PATH, path);
    char fpath[512];
    sprintf(fpath, tpath, NUM_TASK);
    cout << "Start loading STG extra data by reading '" << fpath << "' ...\n";
    ifstream infile(fpath);
    if(!infile) {
        cout <<"error: no such file\n";
        return false;
        // exit(-1);
    }
    // read file and store into memory
    string line, digitstr;
    int i = 0, type = 0;
    int comm;
    while (getline(infile, line)) {
        if(line.empty()) continue;
        if(line.substr(0, 1).compare("*") == 0) continue;
        if(line.substr(0, 1).compare("#") == 0) {
            i = 0;
            ++ type;
            continue;
        }
        int j = 0;
        istringstream iss(line);
        switch (type)
        {
        case 1: // DAG graph
            while(iss >> comm) {
                if(graph[i][j] > 0) 
                    graph[i][j] = comm;
                ++ j;
            }
            break;
        case 2: // task clock cycles
            while(j < m) {
                iss >> task_cc[i][j];
                ++ j;
            } 
            // m = j;
            break;
        
        default:
            cout << "invalid input\n";
            break;
        }
        ++ i;
    }
    infile.close();
    cout << "Complete loading STG extra data!\n";
    return true;
}


#endif