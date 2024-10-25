//
// Created by zeraph on 24‏/1‏/2021.
//

#ifndef herculesHNSW_NODE_H
#define herculesHNSW_NODE_H



#include "globals.h"
#include "Index.h"
#include "Setting.h"
#include "hnswlib/hnswlib.h"
#include "calc_utils.h"
#include "immintrin.h"


typedef hnswlib::HierarchicalNSW<ts_type> TYPEGRAPH2;
class Index;
typedef struct hercules_file_buffer hercules_file_buffer;
/** \Def Z of segment i
 @param   ts_type * indicators; Z_i (min_u_i;max_u_i;min_std_i;max_std_i)
 @param int num_indicators; |indicators|
 * */
typedef struct segment_sketch {
    ts_type *indicators;
    int num_indicators;
} segment_sketch;
/**  \Def store the value and type(mean or std) of threshold used to H_Split
  @param ts_type indicator_split_value;
  @param int indicator_split_idx; //Set Idx = 0 for mean_based split and Idx = 1 for stdev_based split
 * */
typedef struct node_segment_split_policy {
    ts_type indicator_split_value;
    int indicator_split_idx; //Set Idx = 0 for mean_based split and Idx = 1 for stdev_based split
} node_segment_split_policy;
/**
 \Def store info about the seg(hs seg) choosed for the splitting
  @param short split_from; starting point, of the seg or hs seg choosed
  @param short split_to; ending point, of the seg or hs seg choosed
  @param struct node_segment_split_policy curr_node_segment_split_policy; the value and type(mean or std) of threshold used to H_Split
  @param int indicator_split_idx;  // type of threshold used to H_split
  @param ts_type indicator_split_value; // value of threshold
  @param struct segment_sketch series_segment_sketch;
 **/
typedef struct node_split_policy {
    short split_from;
    short split_to;
    node_segment_split_policy curr_node_segment_split_policy;
    int indicator_split_idx;
    ts_type indicator_split_value;
    segment_sketch series_segment_sketch;

} node_split_policy;

class Node {

public:

    Node(Index *index, FILE *file, Node *parent);

    unsigned int node_size;
    unsigned short level;
    unsigned short num_node_points;
    short *node_points;
    static unsigned long num_internal_node ;
    static unsigned long num_leaf_node ;
    static unsigned int max_leaf_size ;
    unsigned long id;
    bool is_hnswed = false;

    node_split_policy *split_policy;

    segment_sketch *node_segment_sketches;

    hercules_file_buffer *file_buffer;
    unsigned char is_leaf;
    char *filename;

    Node *left_child;
    Node *right_child;
    Node *parent;




    TYPEGRAPH2 * leafgraph;

//NEW
    node_segment_split_policy * node_segment_split_policies;
    short unsigned int num_node_segment_split_policies;
    int range = 0 ;
    bool is_left;

    short *hs_node_points;
    segment_sketch *hs_node_segment_sketches;
    short num_hs_node_points;  //number of horizontal split points



    response fileBufferInit();

    void leafToString(unsigned int i);

    void internalToString();



    bool node_split_policy_route_to_left(ts_type *pDouble) const;

    ts_type calculate_node_min_distance(Index *index, ts_type *query, querying_stats & stats);

    static Node *leafNodeInit();

    static Node *rootNodeInit(Setting *pSetting, bool print_info = false);

    void updateStatistics(ts_type *pDouble);

    void addTS(Index * index,const ts_type *pDouble, bool add_in_child = false);

    char *getBufferFullFileName(Index *pIndex) const;


    bool clearFileBuffer(Index *pIndex);


    bool split_node(Index *index, short *pInt, int i);

    bool getOrInitFileBuffer(Index *pIndex);
    ~Node();


    ts_type **getTS(Index *pIndex) const;

    bool deleteFileBuffer(Index *pIndex);
    bool flushFileBuffer(Index * index);




    void write(Index *pIndex, FILE *pFile);

    static Node *Read(Index *pIndex, FILE *pFile);
    char *getLeafGraphFullFileName(Index *index) const;

    void loadGraph(Index *pIndex);

    hnswlib::L2Space *hnswmetric;

    void getLeaves(Node **leaves, int &i);

    void leafToGraph(Index *pIndex);

protected :
    Node();
    Node(Index *index, FILE *file);


    static bool series_segment_sketch_do_sketch(segment_sketch *pSketch, ts_type *pDouble, short from, short to);


    bool node_init_segments(const short *pInt,unsigned short i);

    bool create_node_filename(Setting *pSetting);

    bool node_segment_sketch_update_sketch(segment_sketch *pSketch, ts_type *pDouble, short start, short anEnd);

    Node *create_child_node();
};

struct hercules_file_buffer {

    Node * node; //the buffer points back to its node
    struct hercules_file_map *position_in_map; //the buffer points back to its position in file map

    ts_type **buffered_list;
    unsigned int disk_count; // 0 by default
    int buffered_list_size;   //number of series currently stored in this buffer

    boolean in_disk; //false by default
    boolean do_not_flush;

};
void calc_mean_stdev (ts_type * series, int start, int end, ts_type * mean, ts_type * stdev);
void calc_mean_stdev_per_segment (ts_type * series, short * segments, ts_type *means, ts_type *stdevs, int size);
#ifdef __SSE__
void calc_mean_stdev_SIMD (ts_type * series, int start, int end, ts_type * mean, ts_type * stdev);
__m128 masked_read (int d, const float *x);
__m256 masked_read_8 (int d, const float *x);
void calc_mean_stdev_per_segment_SIMD (ts_type * series, short * segments, ts_type *means, ts_type *stdevs, int size);
#endif
#endif
