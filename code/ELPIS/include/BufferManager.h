//
// Created by zeraph on 23‏/1‏/2021.
//

#ifndef herculesHNSW_BUFFERMANAGER_H
#define herculesHNSW_BUFFERMANAGER_H



#include "Node.h"
#include "Setting.h"
class BufferManager {

public:
    BufferManager(Setting *index_setting);
    unsigned long max_buffered_size;
    long current_count;//#TS points in memory
    struct hercules_file_map *file_map;
    struct hercules_file_map *file_map_tail;
    unsigned int file_map_size;
    unsigned long batch_remove_size;
    char *mem_array;
    int current_record_index;
    long long int max_record_index;
    char *current_record;
    long BudgetMemBytes_;
    void toString();



};
struct hercules_file_map {
    struct hercules_file_buffer *file_buffer;
    struct hercules_file_map *next;
    struct hercules_file_map *prev;
} ;


#endif //herculesHNSW_BUFFERMANAGER_H
