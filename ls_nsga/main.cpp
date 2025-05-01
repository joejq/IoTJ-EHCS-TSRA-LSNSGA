
#include "lsnsga.hpp"

#define RNO 0

int main() {

    /** experiment **/
    int core_num=NUM_CORE, dvfs_num = NUM_DVFS;
    char file_name[128];
    sprintf(file_name, "lsnsga-%d-r%d.dat", NUM_TASK, RNO);
    // STG
    rtg_loading(core_num, dvfs_num, RNO);
    // // PA
    // pa_loading(core_num, dvfs_num);
    run(NUM_RUNS, file_name);

    return 0;

}