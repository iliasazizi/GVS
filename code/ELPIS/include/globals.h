
#ifndef __GLOBAL__
#define __GLOBAL__

//#include "config.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "float.h"
#include <math.h>
//may be included or not
#include <stdbool.h>
#include <omp.h>
//#include <jemalloc/jemalloc.h>
#include "chrono"

#ifdef __SSE__
#define USE_SSE
#define __DO_SSE__
#endif
#define CALC_DC
#define PREFETCH

#define glpq
//#define glpq_clean


#define TIC std::cerr<<"TIC"<<std::endl;
#define TOC std::cerr<<"TOC"<<std::endl;




typedef unsigned short segment_type;
typedef float ts_type;
typedef unsigned long long file_position_type;
typedef unsigned long long root_mask_type;
typedef long long mean_stdev_range;
typedef unsigned int node_id;
typedef unsigned int ts_id;

typedef struct candidate{
    ts_id id_ts;
    ts_type dist;
    candidate(unsigned int y, float z) : id_ts(y),dist(z) {};
    candidate& operator=(const candidate& c){
        id_ts = c.id_ts;
        dist = c.dist;
        return *this;
    }

    size_t pqueue_position;
} candidate;


typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> Time;
inline Time now(){
    return std::chrono::high_resolution_clock::now();
}
inline double getElapsedTime(const Time start_point){
    Time end_point = now();
    auto elapsed = end_point - start_point;
    return elapsed.count();
}


typedef struct querying_stats{
    size_t distance_computations_bsl;
    size_t distance_computations_hrl;
    size_t num_hops_hrl;
    size_t num_hops_bsl;
    size_t distance_computations_lb;
    double time_cnmd;
    double time_leaves_search;
    double time_update_knn;
    double time_routing;
    double time_layer0;
    double time_pq;
    //for workers
    unsigned int num_leaf_checked;
    unsigned int num_leaf_searched ;
    unsigned int num_knn_alters ;
    unsigned int num_candidates;
    void reset(){
        time_cnmd = 0.0;
        time_leaves_search = 0.0;
        time_update_knn = 0.0;
        time_layer0 = 0.0;
        time_routing = 0.0;
        time_pq = 0.0;
        num_hops_hrl=0;
        distance_computations_hrl=0;
        num_hops_bsl=0;
        distance_computations_bsl=0;
        distance_computations_lb=0;
        num_leaf_searched = 0;
        num_knn_alters = 0;
        num_leaf_checked = 0;
        num_candidates=0;
    }

} querying_stats;
struct Comparecandidate{
    constexpr bool operator()(candidate const &a,
                              candidate const &b) const noexcept {
        return a.dist < b.dist;
    }
};





enum response {
    FAILURE = 0, SUCCESS = 1
};
enum insertion_mode {
    PARTIAL = 1,
    TMP = 2,
    FULL = 4,
    NO_TMP = 8
};

enum buffer_cleaning_mode {
    FULL_CLEAN, TMP_ONLY_CLEAN, TMP_AND_TS_CLEAN
};
enum node_cleaning_mode {
    DO_NOT_INCLUDE_CHILDREN = 0,
    INCLUDE_CHILDREN = 1
};


inline void * malloc_index(const size_t size){
    return malloc(size);
}
inline void * malloc_search(const size_t size){
    return malloc(size);
}
typedef bool boolean;



#endif
