//
// Created by zeraph on 23‏/1‏/2021.
//

#ifndef herculesHNSW_INDEX_H
#define herculesHNSW_INDEX_H
#include <unistd.h>
#include <iostream>
#include "stdio.h"
#include "stdlib.h"
#include <dirent.h>
#include "sys/stat.h"
#include "string.h"
#include "Setting.h"
#include "hnswlib/hnswlib.h"
#include "BufferManager.h"
#include "Node.h"
using namespace std;
class Node;
class BufferManager;
class Setting;
typedef struct timelapse timelapse;
class Index {
public:

    Index(char *root_directory, unsigned int mode);
    ~Index();
    Setting *index_setting;
    Node * first_node;
    BufferManager *buffer_manager;
    timelapse * time_stats;

    static Index *
    initIndex(char *path, unsigned int size, double size1, unsigned int segments, unsigned int size2, int construction,
              int m);
    void buildIndexFromBinaryData(char *dataset, file_position_type dataset_size);

    bool addFileBuffer(Node *pNode) const;

    void write();

    static Index *Read(char *path, unsigned int mode);
    unsigned int in_memory;
    int ef;
    hnswlib::L2Space *l2space;

    Node ** getLeaves();

protected :
    Index(char *index_path, unsigned int time_series_size, double buffered_memory_size, unsigned int init_segments,
          unsigned int leaf_size, int efConstruction, int M);


    bool insertTS(ts_type *pDouble);

    char *getIndexFilename() const;
};
struct timelapse {
    double index_building_time;
    double querying_time;
};


#endif //herculesHNSW_INDEX_H
