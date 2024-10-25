//
// Created by zeraph on 24‏/1‏/2021.
//

#include <thread>
#include "Node.h"
#include "hnswlib/hnswlib.h"


using namespace std;

using namespace hnswlib;
unsigned long Node::num_internal_node = 0;
unsigned long Node::num_leaf_node = 0;
unsigned int Node::max_leaf_size = 0;
Node::~Node() {

    for (int i = 0; i < this->num_node_points; ++i) {
        free(this->node_segment_sketches[i].indicators);
    }
    free(node_points);
    free(node_segment_sketches);
    if(is_leaf){
        delete leafgraph;
        delete filename;
    }else{
        delete right_child;
        delete left_child;
    }

};
Node::Node(Index *index, FILE *file, Node * parent) {

    leafgraph = nullptr;

    fread(&is_leaf, sizeof(unsigned char), 1, file);
    fread(&(this->node_size), sizeof(unsigned int), 1, file);
    fread(&(this->level), sizeof(unsigned int), 1, file);

    if (!is_leaf) {
        id = num_internal_node++;
//        this->id = num_internal_node + num_leaf_node;

        this->split_policy = nullptr;
        this->split_policy = static_cast<struct node_split_policy *>(malloc_index(sizeof(node_split_policy)));
        fread(this->split_policy, sizeof(node_split_policy), 1, file);

        fread(&(this->num_node_points), sizeof(short), 1, file);

        this->node_points = nullptr;
        this->node_points = static_cast<short *>(malloc_index(sizeof(short) * this->num_node_points));
        fread(this->node_points, sizeof(short), this->num_node_points, file);
        this->node_segment_sketches = nullptr;
        this->node_segment_sketches = static_cast<segment_sketch *>(malloc_index(
                sizeof(segment_sketch) * this->num_node_points));
        for (int i = 0; i < this->num_node_points; ++i) {
            fread(&(this->node_segment_sketches[i].num_indicators), sizeof(int), 1, file);
            this->node_segment_sketches[i].indicators = nullptr;
            this->node_segment_sketches[i].indicators = static_cast<ts_type *>(malloc_index(
                    sizeof(ts_type) * this->node_segment_sketches[i].num_indicators));
            fread(this->node_segment_sketches[i].indicators, sizeof(ts_type),
                  this->node_segment_sketches[i].num_indicators, file);
        }

        this->file_buffer = nullptr;
        this->is_leaf = 0;
        this->filename = nullptr;


        //this->internalToString();
    }
    else {

//        if(index->in_memory>1)leaves[num_leaf_node++] = this;
//        this->id = num_internal_node + num_leaf_node;

        id = num_leaf_node++;

        this->is_leaf = 1;

//        this->fileBufferInit();
//        this->file_buffer->disk_count = this->node_size;
        this->file_buffer = nullptr;

        int filename_size = 0;
        fread(&filename_size, sizeof(int), 1, file);


        this->is_hnswed = true;


        if (filename_size > 0) {

            this->filename = static_cast<char *>(malloc_index(sizeof(char) * (filename_size + 1)));
            fread(this->filename, sizeof(char), filename_size, file);
            this->filename[filename_size] = '\0';

            fread(&(this->num_node_points), sizeof(short), 1, file);

            this->node_points = nullptr;
            this->node_points = static_cast<short *>(malloc_index(sizeof(short) * this->num_node_points));
            fread(this->node_points, sizeof(short), this->num_node_points, file);


            this->node_segment_sketches = nullptr;
            this->node_segment_sketches = static_cast<segment_sketch *>(malloc_index(
                    sizeof(segment_sketch) * this->num_node_points));

            for (int i = 0; i < this->num_node_points; ++i) {
                fread(&(this->node_segment_sketches[i].num_indicators), sizeof(int), 1, file);
                this->node_segment_sketches[i].indicators = nullptr;
                this->node_segment_sketches[i].indicators = static_cast<ts_type *>(malloc_index(
                        sizeof(ts_type) * this->node_segment_sketches[i].num_indicators));
                fread(this->node_segment_sketches[i].indicators, sizeof(ts_type),
                      this->node_segment_sketches[i].num_indicators, file);
            }
            loadGraph(index);
            if(node_size > max_leaf_size)max_leaf_size = node_size;
        } else {
            this->filename = nullptr;
            this->node_size = 0;
        }
        //this->leafToString(index->index_setting->max_leaf_size);

    }

    this->parent = parent ;
    if (!is_leaf) {
        this->left_child = new Node(index, file, this);
        this->right_child = new Node(index, file, this);
    }
}

enum response Node::fileBufferInit() {
    this->file_buffer = nullptr;
    this->file_buffer = static_cast<hercules_file_buffer *>(malloc(sizeof(hercules_file_buffer)));
    if (this->file_buffer == nullptr) {
        fprintf(stderr, "Error in Node.cpp: Could not allocate memory for file buffer.\n");
        return FAILURE;
    }
    this->file_buffer->in_disk = false;
    this->file_buffer->disk_count = 0;

    this->file_buffer->buffered_list = nullptr;
    this->file_buffer->buffered_list_size = 0;

    this->file_buffer->node = this;
    this->file_buffer->position_in_map = nullptr;

    this->file_buffer->do_not_flush = false;

    return SUCCESS;

}

void Node::leafToGraph(Index *index) {

    if (!this->is_hnswed) {
        ts_type **rec = this->getTS(index);
        if (rec != nullptr) {
            //since we copied value from buffered_file, we can free the memory
//            for(int i=0;i<this->file_buffer->buffered_list_size;i++)free(this->file_buffer->buffered_list[i]);
            free(this->file_buffer->buffered_list);
            this->file_buffer->buffered_list = nullptr;//we still can do better by deleting the file buffer, but no problem for the moment

            char *index_full_filename = this->getLeafGraphFullFileName(index);

            this->hnswmetric = new hnswlib::L2Space(index->index_setting->timeseries_size);

            auto *appr_alg = new HierarchicalNSW<ts_type>(this->hnswmetric, this->node_size, index->index_setting->M,
                                                          index->index_setting->efconstruction);
            appr_alg->addPoint((void *) (rec[0]), (size_t) 0);
            free(rec[0]);
#pragma omp parallel for //num_threads(n2)
                for (int i = 1; i < this->node_size; i++) {
                    appr_alg->addPoint((void *) (rec[i]), (size_t) i);
                    free(rec[i]);
                }

            free(rec);

            appr_alg->saveIndex(index_full_filename);
            delete appr_alg;
            delete hnswmetric;

            this->is_hnswed = true;
            cout << "[Graphing] Leaf " << this->filename << " has been graph-structured in " << index_full_filename
                 << endl;
            free(index_full_filename);
        } else {
            throw std::runtime_error("GetTS() returns nullptr!");
        }
    }
}



bool Node::clearFileBuffer(Index *index) {
    if ((this->file_buffer) == nullptr) {
        fprintf(stderr, "Error in Node.cpp: Cannot clear a NULL buffer.\n");
        return FAILURE;
    } else {
        if (this->file_buffer->buffered_list != nullptr) {
            free(this->file_buffer->buffered_list);
        }

        this->file_buffer->buffered_list = nullptr;
        this->file_buffer->buffered_list_size = 0;
    }

    return SUCCESS;
}


bool Node::node_split_policy_route_to_left(ts_type *query_ts) const {
    segment_sketch series_segment_sketch;
    series_segment_sketch.indicators = nullptr;
    series_segment_sketch.indicators = static_cast<ts_type *>(malloc_search(sizeof(ts_type) * 2));

    if (series_segment_sketch.indicators == nullptr) {
        fprintf(stderr, "Error in Node.cpp: Could not allocate memory for series segment sketch indicators.\n");
        exit(-1);
    }

    series_segment_sketch.num_indicators = 2;

    node_split_policy policy = *this->split_policy;
    boolean route_to_left = false;
    // calc mean and std of segment used in H-split for ts
    if (!series_segment_sketch_do_sketch(&series_segment_sketch, query_ts, policy.split_from, policy.split_to)) {
        fprintf(stderr, "Error in Node.cpp: Could not calculate the series segment sketch .\n");
        exit(-1);
    }


    if (series_segment_sketch.indicators[policy.indicator_split_idx] < policy.indicator_split_value) {
        route_to_left = true;
    }

    free(series_segment_sketch.indicators);
    return route_to_left;

}

bool Node::series_segment_sketch_do_sketch(segment_sketch *pSketch, ts_type *series, short fromIdx, short toIdx) {

#ifdef __DO_SSE__
    calc_mean_stdev_SIMD(series, fromIdx, toIdx, &pSketch->indicators[0], &pSketch->indicators[1]);
#else
    calc_mean_stdev(series, fromIdx, toIdx,&pSketch->indicators[0], &pSketch->indicators[1]);
#endif
    return true;

}



ts_type Node::calculate_node_min_distance(Index *index, ts_type *query, querying_stats & stats) {
//    auto stime = std::chrono::high_resolution_clock::now();
    ts_type temp_dist,temp, sum = 0,tic,toc;
    short *points = this->node_points;
    short start =0;



    auto *mean_per_segment = static_cast<ts_type *>(malloc_search(sizeof(ts_type) * this->num_node_points));
    auto *stdev_per_segment = static_cast<ts_type *>(malloc_search(sizeof(ts_type) * this->num_node_points));

//FASTEST

    for (int i =0 ; i < this->num_node_points; i++) {
#ifdef __DO_SSE__
        calc_mean_stdev_SIMD(query,start, node_points[i], mean_per_segment+i,stdev_per_segment+i);
#else
        calc_mean_stdev(query,start, node_points[i], means+i,stdevs+i);
#endif

        start =  node_points[i];
        //use mean and standard deviation to estimate the distance
        temp_dist = 0;



        tic = stdev_per_segment[i] - this->node_segment_sketches[i].indicators[2];
        toc = stdev_per_segment[i] - this->node_segment_sketches[i].indicators[3];



       if((tic >0) == (toc > 0)) {
            temp = fmin(fabs(tic),fabs(toc));
            temp_dist += temp * temp;

//              temp_dist+=fmin(fabs(tic),fabs(toc));
        }

        tic = mean_per_segment[i] - this->node_segment_sketches[i].indicators[0];
        toc = mean_per_segment[i] - this->node_segment_sketches[i].indicators[1];

        if((tic >0) == (toc > 0)) {
            temp = fmin(fabs(tic),fabs(toc));
            temp_dist += temp * temp;
//            temp_dist+=fmin(fabs(tic),fabs(toc));
        }
        sum += temp_dist * get_segment_length(points, i);
    }



    free(mean_per_segment);
    free(stdev_per_segment);

    return sum;

}


Node *Node::rootNodeInit(Setting *settings, bool print_info) {
    Node *node = leafNodeInit();
    unsigned short ts_length = settings->timeseries_size;
    unsigned short segment_size = settings->init_segments;

    node->node_segment_split_policies = static_cast<node_segment_split_policy *>(malloc_index(
            sizeof(node_segment_split_policy) * 2));

    if (node->node_segment_split_policies == nullptr) {
        cerr << "Error in Node.cpp: Could not allocate memory for root node segment split policies." << endl;
        exit(-1);
    }

    node->node_segment_split_policies[0].indicator_split_idx = 0;  //Mean
    node->node_segment_split_policies[1].indicator_split_idx = 1;  //Stdev
    node->num_node_segment_split_policies = 2;  //Stdev and Mean

    //calc the split points by segmentSize
    short *split_points = nullptr;
    split_points = static_cast<short *>(malloc_index(sizeof(short) * segment_size));
    if (split_points == nullptr) {
        cerr << "Error in Node.cpp: Could not allocate memory for root split points." << endl;
        exit(-1);
    }
    if (!calc_split_points(split_points, ts_length, segment_size)) {
        cerr << "Error in Node.cpp: Could not calculate the split points for the root." << endl;
        exit(-1);
    }

    if (!node->node_init_segments(split_points, segment_size)) {
        cerr << "Error in Node.cpp: Could not initialize the segments for the root." << endl;
        exit(-1);
    }

    if (!node->create_node_filename(settings)) {
        cerr << "Error in Node.cpp: Could not create a filename for the root node." << endl;
        exit(-1);
    }

    free(split_points);

    node->internalToString();
    return node;


}

inline void Node::internalToString() {
    //   auto splitwith = (this->split_policy->indicator_split_idx==0)?"Mean" : "Stdev";
    cout << "[Internal Loading] Node id : " << this->id
         << "; Node size " <<
         this->node_size << "; Level " <<
         this->level << "; Num segments " <<
         this->num_node_points << endl;
}

inline void Node::leafToString(unsigned int leaf_size) {
    cout << "[Leaf Loading] Node id : " << this->id
         << "; Node size " << this->node_size
         << "; Level " << this->level
         << "; Num segments " << this->num_node_points
         << "; Filename " << this->filename
         << "; Ratio " << float(this->node_size) / float(leaf_size)
         << "%" << endl;
}

bool Node::node_init_segments(const short *split_points, unsigned short segment_size) {
    this->num_node_points = segment_size;

    this->node_points = nullptr;
    this->node_points = static_cast<short *>(malloc(sizeof(short) * segment_size));
    if (this->node_points == nullptr) {
        cerr << "Error in node_init_segments(): Could not allocate memory for node points." << endl;
        return FAILURE;
    }

    for (int i = 0; i < segment_size; ++i) {
        this->node_points[i] = split_points[i];
    }

    this->hs_node_points = nullptr;
    this->hs_node_points = static_cast<short *>(malloc(sizeof(short) * segment_size * 2));
    if (this->hs_node_points == nullptr) {
        cerr << "Error in node_init_segments(): Could not allocate memory for hs node points." << endl;
        return FAILURE;
    }

    int min_length = 1; //mininum length of new segment = 1

    calc_hs_split_points(this->hs_node_points, &this->num_hs_node_points, this->node_points, segment_size, min_length);

    //allocate mem for array of sketches


    this->node_segment_sketches = nullptr;
    this->node_segment_sketches = static_cast<segment_sketch *>(malloc(sizeof(segment_sketch) * segment_size));
    if (this->node_segment_sketches == nullptr) {
        cerr << "Error in node_init_segments(): Could not allocate memory for node segment sketches." << endl;
        return FAILURE;
    }


    this->hs_node_segment_sketches = nullptr;
    this->hs_node_segment_sketches = static_cast<segment_sketch *>(malloc(
            sizeof(segment_sketch) * this->num_hs_node_points));
    if (this->hs_node_segment_sketches == nullptr) {
        cerr << "Error in node_init_segments(): Could not allocate memory for hs node segment sketches." << endl;
        return FAILURE;
    }

    //allocate memory for vertical indicators
    for (int i = 0; i < segment_size; ++i) {
        this->node_segment_sketches[i].indicators = nullptr;
        this->node_segment_sketches[i].indicators = static_cast<ts_type *>(malloc(
                sizeof(ts_type) * 4)); //node segment has 4 indicators

        if (this->node_segment_sketches[i].indicators == nullptr) {
            cerr << "Error in node_init_segments(): Could not allocate memory for node segment sketch indicator."
                 << endl;
            return FAILURE;
        }

        this->node_segment_sketches[i].indicators[0] = -FLT_MAX;
        this->node_segment_sketches[i].indicators[1] = FLT_MAX;
        this->node_segment_sketches[i].indicators[2] = -FLT_MAX;
        this->node_segment_sketches[i].indicators[3] = FLT_MAX;
        this->node_segment_sketches[i].num_indicators = 4;
    }//sketches Z init with +/- INF

    //allocate memory for horizontal indicators
    for (int i = 0; i < this->num_hs_node_points; ++i) {
        this->hs_node_segment_sketches[i].indicators = nullptr;
        this->hs_node_segment_sketches[i].indicators = static_cast<ts_type *>(malloc(
                sizeof(ts_type) * 4)); //node segment has 4 indicators
        if (this->hs_node_segment_sketches[i].indicators == nullptr) {
            cerr << "Error in node_init_segments(): Could not allocate memory for hs node segment sketch indicator."
                 << endl;
            return FAILURE;
        }
        this->hs_node_segment_sketches[i].indicators[0] = -FLT_MAX;
        this->hs_node_segment_sketches[i].indicators[1] = FLT_MAX;
        this->hs_node_segment_sketches[i].indicators[2] = -FLT_MAX;
        this->hs_node_segment_sketches[i].indicators[3] = FLT_MAX;
        this->hs_node_segment_sketches[i].num_indicators = 4;
    }  // sketches Z init of each possible 2 new segments after v split

    return SUCCESS;

}

bool Node::create_node_filename(Setting *settings) {
    int i;

    this->filename = static_cast<char *>(malloc(
            sizeof(char) * (settings->max_filename_size)));//max filename size are max size for meta data of leafs

    int l = 0;
    l += sprintf(this->filename + l, "%02d", this->num_node_points);

    // If this has level other than 0 then it is not a root node and as such it does have some
    // split data on its parent.

    if (this->level) {
        if (this->is_left) {
            l += sprintf(this->filename + l, "%s", "_L");
        } else {
            l += sprintf(this->filename + l, "%s", "_R");
        }

        //Add parent split policy, just the code 0 for mean and 1 for stdev
        struct node_split_policy *policy;
        policy = this->parent->split_policy;

        if (policy->indicator_split_idx) {
            l += sprintf(this->filename + l, "%s", "_1");
        } else {
            l += sprintf(this->filename + l, "%s", "_0");
        }

        l += sprintf(this->filename + l, "_(%d,%d,%g)", policy->split_from, policy->split_to,
                     policy->indicator_split_value);


    }

    // If its level is 0 then it is a root
    l += sprintf(this->filename + l, "_%d", this->level);
    l += sprintf(this->filename + l, "_%lu", this->id);

    return SUCCESS;
}

Node *Node::leafNodeInit() {
    Node *node = new Node();
    Node::num_leaf_node++;
    node->id = num_internal_node + num_leaf_node;
    node->is_leaf = 1;
    return node;
}

Node::Node() {
    this->right_child = nullptr;
    this->left_child = nullptr;
    this->parent = nullptr;
    leafgraph = nullptr;
    this->node_segment_split_policies = nullptr;
    this->num_node_segment_split_policies = 2;

    this->range = 0;

    this->level = 0;
    this->is_left = false;
    this->is_leaf = true;

    this->split_policy = nullptr;
    this->node_points = nullptr;
    this->hs_node_points = nullptr;
    this->num_node_points = 0;
    this->num_hs_node_points = 0;

    this->node_segment_sketches = nullptr;
    this->hs_node_segment_sketches = nullptr;

    //  this->max_segment_length = 2;
    //this->max_value_length = 10;

    this->file_buffer = nullptr;

    this->node_size = 0;

}

void Node::updateStatistics(ts_type *ts_new) {

    //update vertical node_segment_sketch
    for (int i = 0; i < this->num_node_points; i++) {
        if (!node_segment_sketch_update_sketch(&this->node_segment_sketches[i], ts_new,
                                               get_segment_start(this->node_points, i),
                                               get_segment_end(this->node_points, i))) {
            fprintf(stderr, "Error in Node.cpp:  Could not update vertical sketch for node segment.\n");
            exit(-1);
        }

    }

    //update horizontal node_segment_sketch
    for (int i = 0; i < this->num_hs_node_points; i++) {
        if (!node_segment_sketch_update_sketch(&this->hs_node_segment_sketches[i], ts_new,
                                               get_segment_start(this->hs_node_points, i),
                                               get_segment_end(this->hs_node_points, i))) {
            fprintf(stderr, "Error in Node.cpp:  Could not update horizontal sketch for node segment.\n");
            exit(-1);
        }

    }

    ++this->node_size;
}

bool Node::node_segment_sketch_update_sketch(struct segment_sketch *node_segment_sketch, ts_type *series, short fromIdx,
                                             short toIdx) {

    segment_sketch series_segment_sketch;

    series_segment_sketch.indicators = nullptr;
    series_segment_sketch.indicators = static_cast<ts_type *>(malloc(sizeof(ts_type) * 2));
    if (series_segment_sketch.indicators == nullptr) {
        fprintf(stderr, "Error in Node.cpp: Could not allocate memory for series segment sketch indicators.\n");
        return FAILURE;
    }
    series_segment_sketch.num_indicators = 2;

    if (!series_segment_sketch_do_sketch(&series_segment_sketch, series, fromIdx, toIdx)) {
        fprintf(stderr, "Error in node_init_segments(): Could not calculate the series segment  sketch.\n");
        return FAILURE;
    }


    if (node_segment_sketch->indicators == nullptr) {
        node_segment_sketch->indicators = static_cast<ts_type *>(malloc(sizeof(ts_type) * 4));
        if (node_segment_sketch->indicators == nullptr) {
            fprintf(stderr,
                    "Error in hercules_node_split.c: Could not allocate memory for node segment sketch indicators.\n");
            return FAILURE;
        }
        node_segment_sketch->indicators[0] = -FLT_MAX; //for max mean
        node_segment_sketch->indicators[1] = FLT_MAX; //for min mean
        node_segment_sketch->indicators[2] = -FLT_MAX; //for max stdev
        node_segment_sketch->indicators[3] = FLT_MAX; //for min stdev
        node_segment_sketch->num_indicators = 4;
    }


    node_segment_sketch->indicators[0] = fmaxf(node_segment_sketch->indicators[0], series_segment_sketch.indicators[0]);
    node_segment_sketch->indicators[1] = fminf(node_segment_sketch->indicators[1], series_segment_sketch.indicators[0]);
    node_segment_sketch->indicators[2] = fmaxf(node_segment_sketch->indicators[2], series_segment_sketch.indicators[1]);
    node_segment_sketch->indicators[3] = fminf(node_segment_sketch->indicators[3], series_segment_sketch.indicators[1]);
    node_segment_sketch->num_indicators = 4;

    free(series_segment_sketch.indicators);
    series_segment_sketch.indicators = nullptr;

    return SUCCESS;
}
void Node::addTS(Index *index, const ts_type *timeseries, bool add_in_child) {

    if (!this->getOrInitFileBuffer(index)) {
        cerr << "Error in Node.cpp:  Could not get the file buffer for node " <<
             this->id << " child of node " << this->parent->id << " " << add_in_child << endl;
        exit(-1);
    }
    if (this->file_buffer == nullptr) {
        cerr << "Error in Node.cpp: node file buffer null after being created !" << add_in_child << endl;
        exit(-1);
    }

    //  cerr << this->getBufferFullFileName(index)<< endl;


    //if id ==0  means buffer list is empty, thus we init buffered list with a pointer to memarray
    int idx = this->file_buffer->buffered_list_size;
    unsigned int ts_length = index->index_setting->timeseries_size;
    unsigned int max_leaf_size = index->index_setting->max_leaf_size;
    if (idx == 0) {
        this->file_buffer->buffered_list = nullptr;
        this->file_buffer->buffered_list = static_cast<ts_type **>(malloc(sizeof(ts_type *) * max_leaf_size));

        if (this->file_buffer->buffered_list == nullptr) {
            cerr << "Error in NOde.cpp:  Could not allocate memory for the buffered list." << add_in_child << endl;
            exit(-1);
        }
    }
    this->file_buffer->buffered_list[idx] = (ts_type *) index->buffer_manager->current_record;//char to *float
    index->buffer_manager->current_record += sizeof(ts_type) * ts_length;
    index->buffer_manager->current_record_index++;
    if (this->file_buffer->buffered_list[idx] == nullptr) {
        cerr << "Error in Node.cpp:  Could not allocate memory for the time series in  the buffer." << add_in_child
             << endl;
        exit(-1);
    }

    for (int i = 0; i < ts_length; ++i) {
        this->file_buffer->buffered_list[idx][i] = timeseries[i];
    }
    ++this->file_buffer->buffered_list_size;
    if (!add_in_child)index->buffer_manager->current_count += ts_length;
}

bool Node::getOrInitFileBuffer(Index *index) {
    if (this->file_buffer == nullptr) {
        if (!this->fileBufferInit()) {
            cerr << "Error in Node.cpp : Couldn't Init file buffer for node " << this->id << endl;
            return FAILURE;
        }

        if (!index->addFileBuffer(this)) {
            cerr << "Error in Node.cpp : Couldn't add file buffer to map for node " << this->id << endl;
            return FAILURE;
        }

    }
    //we do this in case a node will be spliteed and its values will be routed to its childrens
    long long buffer_limit = index->buffer_manager->max_record_index - (2 * index->index_setting->max_leaf_size);
    if (index->buffer_manager->current_record_index > buffer_limit) {
        cout << "Buffer limit " << buffer_limit << ", current recorded in buffer "
             << index->buffer_manager->current_record_index << endl;


        hercules_file_map *currP = index->buffer_manager->file_map;

        while (currP != nullptr) {
            //flush buffer with position idx in the file map of this index
            //fprintf(stderr,"Flushing the buffer"
            //                " for node %s to disk.\n",currP->file_buffer->node->filename);
            if (!currP->file_buffer->node->flushFileBuffer(index)) //flush the actual buffer of the node
            {
                cerr << "Error in Node.cpp : Could not flush the buffer for node " << currP->file_buffer->node->id
                     << ", with filename " << currP->file_buffer->node->filename << ", to disk.";
                return FAILURE;
            }
            currP = currP->next;
        }
        memset(index->buffer_manager->mem_array, 0, index->buffer_manager->max_record_index);
        index->buffer_manager->current_record_index = 0;
        index->buffer_manager->current_record = index->buffer_manager->mem_array;

    }
    return SUCCESS;

}

char *Node::getBufferFullFileName(Index *index) const {
    if (this->filename == nullptr) {
        cerr << "Error in Node.cpp: This node " << this->id
             << " has " << this->file_buffer->disk_count <<
             " data on disk but could not get its filename" << endl;
        return nullptr;
        //return FAILURE;
    }

    unsigned long full_size = strlen(index->index_setting->index_path_hercules) + strlen(this->filename) + 1;
    char *full_filename = static_cast<char *>(malloc_index(sizeof(char) * full_size));
    full_filename = strcpy(full_filename, index->index_setting->index_path_hercules);
    full_filename = strcat(full_filename, this->filename);
    full_filename = strcat(full_filename, "\0");
    return full_filename;
}

char *Node::getLeafGraphFullFileName(Index *index) const {
    if (this->filename == nullptr) {
        cerr << "Error in Node.cpp: This node " << this->id
             << " has " << this->file_buffer->disk_count <<
             " data on disk but could not get its filename" << endl;
        return nullptr;
        //return FAILURE;
    }

    unsigned long full_size = strlen(index->index_setting->index_path_hnsw) + strlen(this->filename) + 4;
    char *full_filename = static_cast<char *>(malloc_index(sizeof(char) * full_size));
    full_filename = strcpy(full_filename, index->index_setting->index_path_hnsw);
    full_filename = strcat(full_filename, this->filename);
    full_filename = strcat(full_filename, ".gl\0");
    return full_filename;
}

bool Node::split_node(Index *index, short *child_node_points, int num_child_node_points) {
    if (!this->is_leaf) {
        cerr << "Error in Node.cpp: Trying to split a node " << this->id << " that is not a leaf " << this->filename
             << endl;
        return FAILURE;
    } else {
        //init children nodes
        //COUNT_LEAF_NODE //only add one leaf since parent was already counted
        Node::num_internal_node += 1;
        Node::num_leaf_node -= 1;

        this->left_child = this->create_child_node();

        if (this->left_child == nullptr) {
            cerr << "Error in Node.cpp: Left child not  initialized properly." << endl;
            return FAILURE;
        }

        if (!this->left_child->node_init_segments(child_node_points, num_child_node_points)) {
            cerr << "Error in Node.cpp: Could not initialize segments for left child of node " << this->id << " | "
                 << this->filename << endl;
            return FAILURE;
        }

        this->left_child->is_leaf = 1;
        this->left_child->is_left = true;
        //the parent node is passed since it has the policy and segments info

        if (!this->left_child->create_node_filename(index->index_setting)) {
            cerr << "Error in Node.cpp: Could not create filename for left child of node " << this->filename << endl;
            return FAILURE;
        }

        this->right_child = this->create_child_node();
        if (this->right_child == nullptr) {
            cerr << "Error in Node.cpp: Left child not  initialized properly." << endl;
            return FAILURE;
        }

        if (!this->right_child->node_init_segments(child_node_points, num_child_node_points)) {
            cerr << "Error in Node.cpp: Could not initialize segments for right child of node " << this->id << " | "
                 << this->filename << endl;
            return FAILURE;
        }

        this->right_child->is_leaf = 1;
        this->right_child->is_left = false;
        //the parent node is passed since it has the policy and segments info

        if (!this->right_child->create_node_filename(index->index_setting)) {
            cerr << "Error in Node.cpp: Could not create filename for right child of node " << this->filename << endl;
            return FAILURE;
        }

        this->is_leaf = 0; //after splitting, node is no longer a leaf
    }
    return SUCCESS;
}

Node *Node::create_child_node() {
    Node *child_node = this->leafNodeInit();

    if (child_node == nullptr) {
        cerr << "Error in Node.cpp: Could not initialize child node of parent " << this->id << " | " << this->filename
             << endl;
        return nullptr;
    }

    child_node->node_segment_split_policies = this->node_segment_split_policies;
    child_node->num_node_segment_split_policies = this->num_node_segment_split_policies;

    child_node->range = this->range;

    child_node->parent = this;

    child_node->level = this->level + 1;

    return child_node;
}

ts_type **Node::getTS(Index *index) const {
    unsigned int ts_length = index->index_setting->timeseries_size;
    unsigned int max_leaf_size = index->index_setting->max_leaf_size;
    auto **ret = static_cast<ts_type **>(calloc(this->node_size, sizeof(ts_type *)));

    if (this->file_buffer->disk_count > 0) {
        char *full_filename = this->getBufferFullFileName(index);

        FILE *ts_file = fopen(full_filename, "r");

        if (ts_file == nullptr) {
            cerr << "Error in Node.cpp: Could not open"
                    "the filename " << full_filename
                 << " Reason = " << full_filename << strerror(errno) << endl;
            return nullptr;
        }

        //in order to keep the same order that the data was inserted, we move the
        //time series that are in memory to allw the disk based time series to be
        //first in the buffer.

        for (int i = 0; i < this->file_buffer->disk_count; ++i) {
            ret[i] = static_cast<ts_type *>(calloc(ts_length, sizeof(ts_type)));
            fread(ret[i], sizeof(ts_type), ts_length, ts_file);
        }

        int idx = this->file_buffer->buffered_list_size;

        for (int i = 0; i < idx; ++i) {
            ret[i + (this->file_buffer->disk_count)] = static_cast<ts_type *>(calloc(ts_length, sizeof(ts_type)));
            for (int j = 0; j < ts_length; ++j) {
                ret[i + (this->file_buffer->disk_count)][j] = this->file_buffer->buffered_list[i][j];
            }
        }

        //this should be equal to old size + disk_count
        //node->file_buffer->buffered_list_size = idx+node->file_buffer->disk_count;
        if (fclose(ts_file)) {
            cerr << "Error in Node.cpp: Could not close"
                    "the filename " << full_filename
                 << ", Reason = " << full_filename << strerror(errno) << endl;
            return nullptr;
        }
        free(full_filename);

    }
    else {
        int idx = this->file_buffer->buffered_list_size;
        for (int i = 0; i < idx; ++i) {
            ret[i] = static_cast<ts_type *>(calloc(ts_length, sizeof(ts_type)));
            for (int j = 0; j < ts_length; ++j) {
                ret[i][j] = this->file_buffer->buffered_list[i][j];
            }
        }
    }
    return ret;
}

bool Node::deleteFileBuffer(Index *index) {
    if (this->file_buffer->in_disk) //delete file if in disk
    {
        //delete file from disk, return and error if not removed properly
        char *full_filename = this->getBufferFullFileName(index);

        if (!remove(full_filename)) //file deleted successfully
        {
            this->file_buffer->disk_count = 0;
            this->file_buffer->in_disk = false;

        } else {
            cerr << "Error in Node.cpp: Error deleting File Buffer filename " << full_filename << endl;
            return FAILURE;
        }
        free(full_filename);
    }

    struct hercules_file_map *res = this->file_buffer->position_in_map;

    if (res != nullptr) {
        if (res->prev == nullptr) //first element in file map
        {
            index->buffer_manager->file_map = res->next;
            if (res->next != nullptr) //deleting the first and there are others elements in map
            {
                res->next->prev = nullptr;
            } else  //deleting first and only element
            {
                index->buffer_manager->file_map_tail = nullptr;
            }
        } else if (res->next == nullptr) //deleting the last element in the map
        {
            res->prev->next = nullptr;
            index->buffer_manager->file_map_tail = res->prev;
        } else {
            res->prev->next = res->next;
            res->next->prev = res->prev;
        }

        free(res);
        --index->buffer_manager->file_map_size;

        if (!this->clearFileBuffer(index)) {
            cerr << "Error in Node.cpp: Deleting node.. "
                    "Could not clear the buffer for " << this->filename << endl;
            return FAILURE;
        }
    }
    cout << "[Buffer Destruction] File Buffer with filename " << this->filename << ", of node " << this->id
         << ", has been deleted!" << endl;

    free(this->file_buffer);
    this->file_buffer = nullptr;

    return SUCCESS;

}

bool Node::flushFileBuffer(Index *index) {
    //is this file flush properly out1/06_R_0_(160,192,0.738156)_9
    if (this->file_buffer->buffered_list_size > 0) {

        if (this->filename == nullptr) {
            cerr << "Error in Index.cpp: Cannot flush the node" << this->id
                 << " to disk. It does not have a filename." << endl;
            return FAILURE;
        }


        char *full_filename = this->getBufferFullFileName(index);

        FILE *ts_file = fopen(full_filename, "a");


        if (ts_file == nullptr) {
            fprintf(stderr, "Error in Index.cpp: Flushing node to disk.."
                            "Could not open the filename %s. Reason= %s\n", this->filename, strerror(errno));
            return FAILURE;
            //return SUCCESS;
        }

        int num_ts = this->file_buffer->buffered_list_size;

        for (int idx = 0; idx < num_ts; ++idx) {

            if (!fwrite(this->file_buffer->buffered_list[idx], sizeof(ts_type), index->index_setting->timeseries_size,
                        ts_file)) {
                fprintf(stderr, "Error in Index.cpp: Could not "
                                "write the timeseries to file %s.\n", full_filename);
                return FAILURE;
            }

        }

        if (fclose(ts_file)) {
            fprintf(stderr, "Error in index.cpp: Flushing node to disk.. "
                            "Could not close the filename %s. Reason = %s.\n", full_filename, strerror(errno));
            return FAILURE;
        }


        this->file_buffer->disk_count += num_ts;

        if (!this->clearFileBuffer(index)) {
            fprintf(stderr, "Error in index.cpp: Flushing node to disk.. "
                            "Could not clear the buffer for %s.\n", full_filename);
            return FAILURE;
        }

        this->file_buffer->in_disk = true;

        free(full_filename);
        cerr << "[FLUSH] Node " << this->id << " buffered " << num_ts << " data has been flushed into disk "
             << this->filename
             << ". Number of total TS in Disk " << this->file_buffer->disk_count << ", Node size " << this->node_size
             << "." << endl;
    }

    return SUCCESS;
}

void Node::write(Index *index, FILE *file) {

    fwrite(&(this->is_leaf), sizeof(unsigned char), 1, file);
    fwrite(&(this->node_size), sizeof(unsigned int), 1, file);
    fwrite(&(this->level), sizeof(unsigned int), 1, file);


    if (this->is_leaf) {

        if (this->filename != nullptr) {
            printf("[Save Leaf]  id : %i | size : %i\n",num_leaf_node++,this->node_size);
            //    cerr << "[TEST]"<< this->filename<<endl;
            int filename_size = strlen(this->filename);
            fwrite(&filename_size, sizeof(int), 1, file);
            fwrite(this->filename, sizeof(char), filename_size, file);


            fwrite(&(this->num_node_points), sizeof(short), 1, file);
            fwrite(this->node_points, sizeof(short), this->num_node_points, file);

            for (int i = 0; i < this->num_node_points; ++i) {

                fwrite(&(this->node_segment_sketches[i].num_indicators),
                       sizeof(int),
                       1,
                       file);
                fwrite(this->node_segment_sketches[i].indicators,
                       sizeof(ts_type),
                       this->node_segment_sketches[i].num_indicators,
                       file);

            }

            //collect stats while traversing the index


        } else {
            int filename_size = 0;
            fwrite(&filename_size, sizeof(int), 1, file);
        }
    } else {

        fwrite(this->split_policy, sizeof(struct node_split_policy), 1, file);
        fwrite(&(this->num_node_points), sizeof(short), 1, file);
        fwrite(this->node_points, sizeof(short), this->num_node_points, file);
        for (int i = 0; i < this->num_node_points; ++i) {

            fwrite(&(this->node_segment_sketches[i].num_indicators),
                   sizeof(int),
                   1,
                   file);
            fwrite(this->node_segment_sketches[i].indicators,
                   sizeof(ts_type),
                   this->node_segment_sketches[i].num_indicators,
                   file);

        }
    }

    if (!this->is_leaf) {
        this->left_child->write(index, file);
        this->right_child->write(index, file);
    }

}

Node *Node::Read(Index *pIndex, FILE *pFile) {
    return new Node(pIndex, pFile, nullptr);
}

void Node::loadGraph(Index * index) {
    if(is_leaf and leafgraph == nullptr) {
        char *index_full_filename = this->getLeafGraphFullFileName(index);
        this->leafgraph = new hnswlib::HierarchicalNSW<ts_type>(index->l2space, index_full_filename,index->ef);
        free(index_full_filename);
    }
}

void Node::getLeaves(Node **leaves, int &i) {
    if(is_leaf){

        leaves[i++] = this;
    }else{
        left_child->getLeaves(leaves,i);
        right_child->getLeaves(leaves,i);
    }
}


void calc_mean_stdev (ts_type * series, int start, int end, ts_type * mean, ts_type * stdev)
{
    ts_type sum_x_squares=0, sum_x=0; //sum of x's and sum of x squares
    int i, count_x;

    *stdev = 0;
    *mean = 0;

    if (start >= end)
    {
        printf ("error in stdev start >= end\n");
    }
    else
    {
        count_x = end-start; //size of the series

        for (int i=start; i<end; i++)
        {
            sum_x += series[i];
            sum_x_squares += series[i] * series[i];
        }

        *mean = sum_x/count_x;
//        sum_x_squares -= ((sum_x * sum_x) / count_x);

        //DO WE REALLY NEED SQRT???
        *stdev = sqrt(sum_x_squares/count_x);

//      *mean = sum_x;
//      *stdev -= ((sum_x * sum_x) / count_x);
    }

}


#ifdef __DO_SSE__
void calc_mean_stdev_SIMD (ts_type * series, int start, int end, ts_type * mean, ts_type * stdev)
{
    float sum_x_squares=0, sum_x=0; //sum of x's and sum of x squares
    int i, count_x;
    __m256 v_t,v_s,v_d,v_sum, v_sum_squares;
    __m128 v_t_128,v_s_128,v_d_128,v_sum_128, v_sum_squares_128;

    float sumf[8], sum_squaresf[8];

    *stdev = 0;
    *mean = 0;

    if (start >= end)
    {
        printf ("error in stdev start >= end\n");
    }
    else
    {
        count_x = end-start; //size of the series

        int d = end-start;
        ts_type * x = series + start;

        while (d >= 8)
        {
            v_t=_mm256_loadu_ps (x); x = x + 8;

            v_sum = _mm256_hadd_ps (v_t, v_t);
            v_sum = _mm256_hadd_ps (v_sum, v_sum);
            _mm256_storeu_ps (sumf ,v_sum);
            sum_x +=sumf[0]+sumf[4];

            v_s=_mm256_mul_ps (v_t,v_t);
            v_sum_squares = _mm256_hadd_ps (v_s, v_s);
            v_sum_squares = _mm256_hadd_ps (v_sum_squares, v_sum_squares);
            _mm256_storeu_ps (sum_squaresf ,v_sum_squares);
            sum_x_squares +=sum_squaresf[0]+sum_squaresf[4];

            d -= 8;
        }

        if (d > 0)
        {
            v_t = masked_read_8 (d,x);

            v_sum = _mm256_hadd_ps (v_t, v_t);
            v_sum = _mm256_hadd_ps (v_sum, v_sum);
            _mm256_storeu_ps (sumf ,v_sum);
            sum_x +=sumf[0]+sumf[4];

            v_s=_mm256_mul_ps (v_t,v_t);
            v_sum_squares = _mm256_hadd_ps (v_s, v_s);
            v_sum_squares = _mm256_hadd_ps (v_sum_squares, v_sum_squares);
            _mm256_storeu_ps (sum_squaresf ,v_sum_squares);
            sum_x_squares +=sum_squaresf[0]+sum_squaresf[4];

        }

        *mean = sum_x/count_x;
        sum_x_squares -= ((sum_x * sum_x) / count_x);

        *stdev = sqrt(sum_x_squares/count_x);
//    *stdev = sum_x_squares/count_x;
//    *mean = sum_x/count_x;

    }

}

__m128 masked_read (int d, const float *x)
{
    assert (0 <= d && d < 4);
    __attribute__((__aligned__(16))) float buf[4] = {0, 0, 0, 0};
    switch (d) {
        case 3:
            buf[2] = x[2];
        case 2:
            buf[1] = x[1];
        case 1:
            buf[0] = x[0];
    }
    return _mm_load_ps (buf);
    // cannot use AVX2 _mm_mask_set1_epi32
}

__m256 masked_read_8 (int d, const float *x)
{
    assert (0 <= d && d < 8);
    __attribute__((__aligned__(32))) float buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    switch (d) {
        case 7:
            buf[6] = x[6];
        case 6:
            buf[5] = x[5];
        case 5:
            buf[4] = x[4];
        case 4:
            buf[3] = x[3];
        case 3:
            buf[2] = x[2];
        case 2:
            buf[1] = x[1];
        case 1:
            buf[0] = x[0];
    }
    return _mm256_load_ps (buf);
    // cannot use AVX2 _mm_mask_set1_epi32
}

void calc_mean_stdev_per_segment_SIMD (ts_type * series, short * segments, ts_type *means, ts_type *stdevs, int size)
{
    int start=0, end;
    for (int i=0; i< size; i++)
    {
        end = segments[i];
        calc_mean_stdev_SIMD (series, start, end,&means[i],&stdevs[i]);
        start = end;
    }
}
#endif