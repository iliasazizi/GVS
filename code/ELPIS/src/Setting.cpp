//
// Created by zeraph on 23‏/1‏/2021.
//


#include "Setting.h"

Setting::Setting(const char *index_path_hercules, const char *index_path_hnsw, unsigned int timeseries_size,
                 unsigned int init_segments, unsigned int max_leaf_size, double buffered_memory_size,
                 int efConstruction, int M) {
    std::cout << "Create Index Setting!" << std::endl;
    this ->index_path_hercules = index_path_hercules;
    this ->index_path_hnsw = index_path_hnsw;
    this ->timeseries_size = timeseries_size;
    this ->init_segments = init_segments;
    this ->max_leaf_size = max_leaf_size;
    this ->buffered_memory_size = buffered_memory_size;
    this ->efconstruction = efConstruction;
    this ->M = M ;

    // ceil == partie entiere of a float + 1, or return the same number if its a int
    float segment_ends_size = ceil(
            log10(SHRT_MAX));//shrt_max is max value of short int, return x s.t : 10^x = 32767 // 2bytes for shrt int
    float split_value_size = ceil(
            log10(INT_MAX) + 1);//int_max is max value of an int, 10^x = 2147483647 // 4bytes for int

    this->max_filename_size = 2 + 1 + 1 +
                                  2 * (segment_ends_size) +
                                  10 + split_value_size +
                                  10 + ceil(log10(LONG_MAX))+//for node id
                                  8 + 1;//nu,ber of punctuation marks + null char


    this -> toString();
}


void Setting::toString(){
    std::cout << "[Index Setting] Path :" << this->index_path_hnsw
    << " | TS Length : " << this->timeseries_size <<
    " | LEAF size : "<< this->max_leaf_size << " | Limit of Usable memory(MB) : "<<
    this->buffered_memory_size << " | M : "<< this->M<< " | efConstruction : "<< this->efconstruction<<std::endl;
}