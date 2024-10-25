//
// Created by zeraph on 25‏/1‏/2021.
//

#ifndef herculesHNSW_QUERYENGINE_H
#define herculesHNSW_QUERYENGINE_H

#include "Index.h"
#include "globals.h"
#include "hnswlib/hnswlib.h"
#include "pqueue.h"
#include <queue>
#include "future"

typedef struct query_result query_result;

typedef struct bsf_snapshot bsf_snapshot;

typedef struct q_index q_index;

typedef struct query_worker_data  worker_backpack__;

struct CompareByFirst{
    constexpr bool operator()(std::pair<float, unsigned int> const &a,std::pair<float, unsigned int> const &b) const noexcept {
        return a.first < b.first;
    }
};

class QueryEngine {
public :
    QueryEngine();

    QueryEngine(const char *query_filename, Index *index, int ef, unsigned int nprobes, bool parallel,
                unsigned int nworker, bool flatt, int k);

    Index * index;
    const char * query_filename;
    unsigned int nprobes;
    queue<unsigned int> visited;
    bool parallel;
    unsigned int nworker;
    std::priority_queue<std::pair<float,unsigned int>, std::vector<std::pair<float,unsigned int>>> top_candidates;
    FILE *query_file;

    querying_stats stats;
    float *results;
    worker_backpack__ *qwdata;
    pqueue_t *pq;
    pqueue_t *candidate_leaves;
    void closeFile();

    file_position_type total_records;



    void queryBinaryFile(int q_num, unsigned int k, int i);

    void printKNN(float *results, int k, double time, queue<unsigned int> &visited, bool para=0);

    void searchNpLeafParallel(ts_type *query_ts, unsigned int k, unsigned int nprobes);

    void setEF(Node *node, int ef);

    inline void copypq(query_worker_data *pData, priority_queue<pair<float, unsigned int>> queue);


    ~QueryEngine();



    unsigned short *flags;
    unsigned short curr_flag;
};


/**
  @param ts_type distance;
  @param struct hercules_node *node;
  @param ts_type max_distance;
  @param size_t pqueue_position;//unsigned long
 */
struct query_result {
    ts_type distance;
    Node *node;
    hnswlib::labeltype ts_num;
    ts_type max_distance;
    size_t pqueue_position;
};
/**
  @param ts_type distance;
  @param double time;
*/
struct bsf_snapshot {
    ts_type distance;
    double time;
} ;

/** Data structure for sorting the query.
  @param double value;
  @param      int  index;
 */
struct q_index {
    double value;
    int index;
} ;

static int cmp_pri(double next, double curr) {
    return (next > curr);
}



static double
get_pri(void *a) {
    return (double) ((struct query_result *) a)->distance;
}
static double
get_max_pri(void *a) {
    return (double) ((struct query_result *) a)->max_distance;
}

static void
set_pri(void *a, double pri) {
    ((struct query_result *) a)->distance = (float) pri;
}

static void
set_max_pri(void *a, double pri) {
    ((struct query_result *) a)->max_distance = (float) pri;
}


static size_t
get_pos(void *a) {
    return ((struct query_result *) a)->pqueue_position;
}


static void
set_pos(void *a, size_t pos) {
    ((struct query_result *) a)->pqueue_position = pos;
}


static double
get_pri2(void *a) {
    return (double) ((candidate *) a)->dist;
}
static void
set_pri2(void *a, double pri) {
    ((candidate *) a)->dist = (float) pri;
}


static size_t
get_pos2(void *a) {
    return ((candidate  *) a)->pqueue_position;
}


static void
set_pos2(void *a, size_t pos) {
    ((candidate *) a)->pqueue_position = pos;
}
//





typedef struct query_worker_data
{
    std::priority_queue<std::pair<float,unsigned int>, std::vector<std::pair<float,unsigned int>>> * top_candidates;
    ts_type * kth_bsf;
    int id;
    querying_stats * stats;
    float * localknn;
    float bsf;
    unsigned short local_nprobes;
    bool end;
    unsigned short *flags;
    unsigned short curr_flag;

} worker_backpack__;












#endif //herculesHNSW_QUERYENGINE_H
