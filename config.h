/**
 * author: Qiangq Jiang
 * date: 2024.12.06
 * 
 * User input
**/
#ifndef CONFIG_H
#define CONFIG_H

#define NUM_SETTING_CORE 5 // 5, 10, 15, 20
#define RA_NO 2  // 0: fpppp-task=334  1: robot-task=88  2: sparse-task=96
#define NUM_TASK 50
#define NUM_RUNS 20
#define NUM_CASE 10
#define BATCH_CAPACITY 200
#define MAX_N (NUM_TASK + 10) // the max number of tasks

#define PRE_STG_PATH "../../../benchmark/stg/%d/"
#define PRE_STG_EXTRA_PATH "../../../benchmark/stg_extra/%d/"
#define PRE_BENCH_PATH "../benchmark/"

/*Algorithm*/
#define LSNSGA_PATH "ls_nsga/"

/*Different output direct*/
#define PRE_OUT_PATH "out/"
#define OUT_PATH "out/"
#define RESULT_PATH "result/"
#define PARETO_PATH "ps/"
#define OBJ_PATH "obj/"
#define DEFAULT_FILE "out.dat"
#endif