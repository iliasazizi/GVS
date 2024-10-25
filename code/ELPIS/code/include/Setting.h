//
// Created by zeraph on 23‏/1‏/2021.
//

#ifndef herculesHNSW_SETTING_H
#define herculesHNSW_SETTING_H
#include "iostream"
#include <climits>
#include "globals.h"
class Setting {
public:
    Setting(const char *index_path_hercules, const char *index_path_hnsw, unsigned int timeseries_size,
            unsigned int init_segments, unsigned int max_leaf_size, double buffered_memory_size,
            int i, int i1);
    void toString();
    const char *index_path_hercules;
    const char *index_path_hnsw;
    unsigned short timeseries_size;
    unsigned short init_segments;
    unsigned int max_leaf_size;
    double buffered_memory_size;
    unsigned short max_filename_size;
    int efconstruction;
    int M;
};


#endif //herculesHNSW_SETTING_H
