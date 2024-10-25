//
// Created by zeraph on 23‏/1‏/2021.
//

#include "Index.h"

/**
 Construct Index object from Index file :<br>
 <ul>
 <li> Read setting meta data from root file, initiate setting object</li>
 <li> Read buffer manager meta data from setting, initiate buffer_manager object</li>
 <li>read nodes data from files</li>
 * */


/**
 <center><h3>Init Index => Setting and BufferManager and root node</h3>
 */
Index * Index::initIndex(char *path, unsigned int size, double size1, unsigned int segments, unsigned int size2,
                         int construction,
                         int m) {

    return new Index(path, size, size1, segments, size2, construction, m);

}

Index::Index(char *index_path, unsigned int timeseries_size, double buffered_memory_size,
             unsigned int init_segments, unsigned int max_leaf_size, int efConstruction, int M){
    this->time_stats = new timelapse ;
    this->time_stats->index_building_time = 0;
    this->time_stats->querying_time = 0;
    in_memory = 0;
    if(chdir(index_path) == 0)
        throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");


mkdir(index_path , 07777);
    char * index_path_hercules = static_cast<char *>(malloc(sizeof(char) * (strlen(index_path) + 4)));
    index_path_hercules = strcpy( index_path_hercules , index_path);
    index_path_hercules  = strcat( index_path_hercules , "tmp/");
    mkdir(index_path_hercules,0777);

    cout << index_path_hercules << endl;

    this -> index_setting =new Setting(index_path_hercules,index_path, timeseries_size, init_segments, max_leaf_size, buffered_memory_size, efConstruction, M);
    this->buffer_manager = new BufferManager(this->index_setting);
    this->first_node = Node::rootNodeInit(this->index_setting);


}

/**
 * \DEF : build the index using timeseries in dataset
 *  \DO1 : check the number of ts in dataset is correct,
 *  \DO2 : iteratively build the index using hercules_index_insert()
 *
 *   <hr>
 *  <h2><center>hercules_index_insert </center></h2>
 *  <p>starting from the root node, while routing to corresponding leaf node do</p>
\DO1 update internal nodes statistics, sketches and hs sketch if the new ts have min/max mean/std values for each segment
\DO2  routing ts with node_split_policy_to_left(), using the split policy of the internal node
\DO3 In the leaf node, init a hercules_file_buffer and to the linked list hercules_file_map if necessary;
\DO4 store ts to mem_array, and its & in file_buffer->buffered_list, update the other informations
\DO5 <b>IF node->size > setting->max_leaf_size :</b> <i> Init node_split_policy, set B to -INF</i><br>
   for every segment i calculate its QOS and Qos of its children using for every possible split, then calculate the B_i and check if its > B;<br>
        <b> Choose split_strategy that gives max B
        <br> IF HS is selected, children node will have additional node_point
\DO6 Create child nodes and init them with specific information and new node_point
 \DO7 get_file_buffer() for new nodes, and also to flush index buffer to disk if necessary(limit)
 \DO8 copy parent node ts(either they are on disk, in memory, or both), and route them to children
\DO9 deallocate space occupied by parent's buffer filename and hercules_file_buffer, and delete it from buffer_file_map linkedlist

   \main_idea_of_splitting maximize difference between QOS of parent and QOS of new children, hmmm basically we try to minimize QOS (len*(difference_between_minandmax_mean² + max_std²)) of leaves, so we select the splitting strategy giving the max diff between qos of parent and the avg qos of its two new children
<br> <b>range=Qos</b>
 * */
void Index::buildIndexFromBinaryData(char * dataset, file_position_type dataset_size) {
    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    auto * ts = static_cast<ts_type *>(malloc(sizeof(ts_type) * this->index_setting->timeseries_size));
    if(ts == nullptr){
        cerr << "Could not Allocate ts in index.cpp"<<endl;
        exit(-1);
    }

    FILE *ifile;
    ifile = fopen(dataset   , "rb");
    if (ifile == nullptr) {
        fprintf(stderr, "Error in index.c: File %s not found!\n", dataset);
        exit(-1);
    }

    fseek(ifile, 0L, SEEK_END);
    auto sz = (file_position_type) ftell(ifile);
    file_position_type total_records = sz /this->index_setting->timeseries_size * sizeof(ts_type);
    fseek(ifile, 0L, SEEK_SET);
    if (total_records < dataset_size) {
        fprintf(stderr, "File %s has only %llu records!\n", dataset, total_records);
        exit(-1);
    }

    file_position_type ts_loaded = 0;

    while (ts_loaded < dataset_size) {

        //printf("\r\x1b[32mLoading: \x1b[36m%d\x1b[0m\n",(ts_loaded + 1));

        fread(ts, sizeof(ts_type), this->index_setting->timeseries_size, ifile);

        if (!this->insertTS(ts)) {
            cerr << "Error in index.c:  Could not add the time series to the index."<<endl;
            exit(-1);
        }
        ts_loaded++;
    }

    hercules_file_map *lastP = this->buffer_manager->file_map_tail;
    Node ** nodes = static_cast<Node **>(malloc_index(Node::num_leaf_node * sizeof(Node *)));
    int i =0;
    while(lastP != nullptr){
            nodes[i++] = lastP->file_buffer->node;
            lastP = lastP->prev;
        }

#pragma omp parallel default(none) shared(i,nodes)
    {
#pragma omp for //num_threads(n1)
        for(int j =0; j<i;j++)
            nodes[j]->leafToGraph(this);
    }

    for(int j =0; j<i;j++)
        nodes[j]->deleteFileBuffer(this);
    free(this->buffer_manager->mem_array);

    free(ts);
    if (fclose(ifile)) {
        fprintf(stderr, "Error in index.cpp: Could not close the filename %s", dataset);
        exit(-1);
    }



// Record end time
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    this->time_stats->index_building_time = elapsed.count();

}

/**
 <ol>
 <li>IF node is not leaf, root TS using policies and node & new ts Sketches until to reach the adequate leaf</li>
 <li>IF node reaches is leaf, add ts to the node
    <ul><li>check if node has buffer file if not init it and map it</li>
        <li>check if memarray is nearly full, if so flush data to disk</li>
        <li>copy @  where the new ts will be registrated at memarray in buffered list, and copy new ts in mem array</li>
        </ul>
 <li>IF leaf node has exceed number of permitted ts to save, we split the into two leaf</li></ol>

   \main_idea_of_splitting maximize difference between QOS of parent and QOS of new children,
   hmmm basically we try to minimize QOS (len*(difference_between_minandmax_mean² + max_std²)) of leaves,
   so we select the splitting strategy giving the max diff between qos of parent and the avg qos of
   its two new children
<br> <b>range=Qos</b>

        */
bool Index::insertTS(ts_type *ts_new) {
    Node * node = this->first_node;

    while(!node->is_leaf){
        node->updateStatistics(ts_new);

        if (node->node_split_policy_route_to_left(ts_new))
            node = node->left_child;
        else
            node = node->right_child;
    }


    if(node->is_leaf){

        node->updateStatistics(ts_new);


        node->addTS(this,ts_new);


        if (node->node_size >= this->index_setting->max_leaf_size) {
            node_split_policy curr_node_split_policy;
            ts_type max_diff_value = (FLT_MAX *(-1));//set init B to -INF,
            // and try to find the max split strategy that maximize B
            ts_type avg_children_range_value;
            short hs_split_point = -1;
            short *child_node_points;
            int num_child_node_points;
            const int num_child_segments = 2; //by default split to two subsegments

            for (int i = 0; i < node->num_node_points; ++i) {
                segment_sketch curr_node_segment_sketch = node->node_segment_sketches[i];

                //This is the QoS of this segment. QoS is the estimation quality evaluated as =
                //QoS = segment_length * (max_mean_min_mean) * ((max_mean_min_mean) +
                //     (max_stdev * max_stdev))
                //The smaller the QoS, the more effective the bounds are for similarity
                //estimation

                ts_type node_range_value = calculate_mean_std_dev_range(curr_node_segment_sketch, get_segment_length(node->node_points, i));

                //for every split policy
                for (int j = 0; j < node->num_node_segment_split_policies; ++j) {
                    struct node_segment_split_policy curr_node_segment_split_policy =
                            node->node_segment_split_policies[j];
                    //to hold the two child segments


                    auto child_node_segment_sketches = static_cast<segment_sketch *>(malloc(
                            sizeof(struct segment_sketch) * num_child_segments));

                    if (child_node_segment_sketches == nullptr) {
                        cerr <<"Error in Index.cpp: could not allocate memory for the child node segment sketches for node "
                                  << node->filename << endl;
                        return FAILURE;
                    }

                    for (int k = 0; k < num_child_segments; ++k) {
                        child_node_segment_sketches[k].indicators = nullptr;
                        child_node_segment_sketches[k].indicators = static_cast<ts_type *>(malloc(
                                sizeof(ts_type) * curr_node_segment_sketch.num_indicators));
                        if (child_node_segment_sketches[k].indicators == nullptr) {
                            cerr << "Error in Index.cpp: could not allocate memory for the child node segment sketches "
                                    "indicators for node " <<node->filename<<endl;
                            return FAILURE;
                        }

                    }


                    if (is_split_policy_mean(curr_node_segment_split_policy))
                        mean_node_segment_split_policy_split(&curr_node_segment_split_policy,
                                                             curr_node_segment_sketch,
                                                             child_node_segment_sketches);
                    else if (is_split_policy_stdev(curr_node_segment_split_policy))
                        stdev_node_segment_split_policy_split(&curr_node_segment_split_policy,
                                                              curr_node_segment_sketch,
                                                              child_node_segment_sketches);
                    else {
                        cerr << "Error in Index.cpp: Split policy was not set properly for node"<< node->filename<<endl;
                        return FAILURE;
                    }

                    ts_type range_values[num_child_segments];
                    for (int k = 0; k < num_child_segments; ++k) {
                        struct segment_sketch child_node_segment_sketch = child_node_segment_sketches[k];
                        range_values[k] = calculate_mean_std_dev_range(child_node_segment_sketch,
                                                     get_segment_length(node->node_points, i));
                    }

                    //diff_value represents the splitting benefit
                    //B = QoS(N) - (QoS_leftNode + QoS_rightNode)/2
                    //the higher the diff_value, the better is the splitting

                    avg_children_range_value = calc_mean(range_values, 0, num_child_segments);
                    ts_type diff_value = node_range_value - avg_children_range_value;

                    if (diff_value > max_diff_value) {
                        max_diff_value = diff_value;
                        curr_node_split_policy.split_from = get_segment_start(node->node_points, i);
                        curr_node_split_policy.split_to = get_segment_end(node->node_points, i);
                        curr_node_split_policy.indicator_split_idx = curr_node_segment_split_policy.indicator_split_idx;
                        curr_node_split_policy.indicator_split_value = curr_node_segment_split_policy.indicator_split_value;
                        curr_node_split_policy.curr_node_segment_split_policy = curr_node_segment_split_policy;
                    }
                    for (int k = 0; k < num_child_segments; ++k) {
                        free(child_node_segment_sketches[k].indicators);
                    }
                    free(child_node_segment_sketches);
                }
            }

            //add trade-off for horizontal split,
            // we bias for minimize number if segments by giving preference to H split more than V split
            max_diff_value = max_diff_value * 2;

            //we want to test every possible split policy for each horizontal segment
            for (int i = 0; i < node->num_hs_node_points; ++i) {
                struct segment_sketch curr_hs_node_segment_sketch = node->hs_node_segment_sketches[i];
                ts_type node_range_value = calculate_mean_std_dev_range(curr_hs_node_segment_sketch,
                                                      get_segment_length(node->hs_node_points, i));

                //for every split policy
                for (int j = 0; j < node->num_node_segment_split_policies; ++j) {
                    struct node_segment_split_policy curr_hs_node_segment_split_policy = node->node_segment_split_policies[j];

                     //to hold the two child segments
                    auto child_node_segment_sketches = static_cast<segment_sketch *>(malloc(sizeof(struct segment_sketch) *
                                                                                       num_child_segments));
                    if (child_node_segment_sketches == nullptr) {
                        cerr <<"Error in INdex.cpp: could not allocate memory \
                            for the horizontal child node segment sketches for \
                            node " << node->filename<<endl;
                        return FAILURE;
                    }

                    for (int k = 0; k < num_child_segments; ++k) {
                        child_node_segment_sketches[k].indicators = nullptr;
                        child_node_segment_sketches[k].indicators = static_cast<ts_type *>(malloc(
                                sizeof(ts_type) * curr_hs_node_segment_sketch.num_indicators));
                        if (child_node_segment_sketches[k].indicators == nullptr) {
                            cerr <<"Error in INdex.cpp: could not allocate memory \
                            for the horizontal child node segment sketches indicatores for \
                            node " << node->filename<<endl;
                            return FAILURE;
                        }
                    }

                    if (is_split_policy_mean(curr_hs_node_segment_split_policy))
                        mean_node_segment_split_policy_split(&curr_hs_node_segment_split_policy,
                                                             curr_hs_node_segment_sketch,
                                                             child_node_segment_sketches);
                    else if (is_split_policy_stdev(curr_hs_node_segment_split_policy))
                        stdev_node_segment_split_policy_split(&curr_hs_node_segment_split_policy,
                                                              curr_hs_node_segment_sketch,
                                                              child_node_segment_sketches);
                    else
                        printf("split policy not initialized properly\n");

                    ts_type range_values[num_child_segments];
                    for (int k = 0; k < num_child_segments; ++k) {
                        struct segment_sketch child_node_segment_sketch = child_node_segment_sketches[k];
                        range_values[k] = calculate_mean_std_dev_range(child_node_segment_sketch,
                                                     get_segment_length(node->hs_node_points, i));
                    }

                    avg_children_range_value = calc_mean(range_values, 0, num_child_segments);

                    ts_type diff_value = node_range_value - avg_children_range_value;

                    if (diff_value > max_diff_value) {
                        max_diff_value = diff_value;
                        curr_node_split_policy.split_from = get_segment_start(node->hs_node_points, i);
                        curr_node_split_policy.split_to = get_segment_end(node->hs_node_points, i);
                        curr_node_split_policy.indicator_split_idx = curr_hs_node_segment_split_policy.indicator_split_idx;
                        curr_node_split_policy.indicator_split_value = curr_hs_node_segment_split_policy.indicator_split_value;
                        curr_node_split_policy.curr_node_segment_split_policy = curr_hs_node_segment_split_policy;
                        hs_split_point = get_hs_split_point(node->node_points,
                                                            curr_node_split_policy.split_from,
                                                            curr_node_split_policy.split_to,
                                                            node->num_node_points);
                    }

                    for (int k = 0; k < num_child_segments; ++k) {
                        free(child_node_segment_sketches[k].indicators);
                    }

                    free(child_node_segment_sketches);
                }
            }

            // we create node->split_policy for the choosen split policy
            node->split_policy = nullptr;
            node->split_policy = static_cast<node_split_policy *>(malloc(sizeof(struct node_split_policy)));
            if (node->split_policy == nullptr) {
                cerr <<"Error in Index.cpp: could not allocate memory \
                        for the split policy of node  "<< node->filename<<endl;
                return FAILURE;
            }
            node->split_policy->split_from = curr_node_split_policy.split_from;
            node->split_policy->split_to = curr_node_split_policy.split_to;
            node->split_policy->indicator_split_idx = curr_node_split_policy.indicator_split_idx;
            node->split_policy->indicator_split_value = curr_node_split_policy.indicator_split_value;
            node->split_policy->curr_node_segment_split_policy = curr_node_split_policy.curr_node_segment_split_policy;


            //when hs_split_point stays less than 0, it means that
            //considering splitting a vertical segment is not worth it
            //according to the QoS heuristic

            if (hs_split_point < 0) {
                num_child_node_points = node->num_node_points;
                child_node_points = static_cast<short *>(malloc(sizeof(short) * num_child_node_points));
                if (child_node_points == nullptr) {
                   cerr<<"Error in Index.cpp: could not allocate memory "
                         "for the child node segment points node  " <<node->filename<<endl;
                    return FAILURE;
                }
                //children will have the same number of segments as parent
                for (int i = 0; i < num_child_node_points; ++i) {
                    child_node_points[i] = node->node_points[i];
                }
            }
            else {
                num_child_node_points = node->num_node_points + 1;
                child_node_points = static_cast<short *>(malloc(sizeof(short) * num_child_node_points));
                if (child_node_points == nullptr) {
                    cerr <<"Error in Index.c: could not allocate memory for the child node segment points node  "<< node->filename<<endl;
                    return FAILURE;
                }
                //children will have one additional segment than the parent
                for (int i = 0; i < (num_child_node_points - 1); ++i) {
                    child_node_points[i] = node->node_points[i];
                }
                child_node_points[num_child_node_points - 1] = hs_split_point; //initialize newly added point

                qsort(child_node_points, num_child_node_points, sizeof(short),
                      reinterpret_cast<__compar_fn_t>(compare_short));

            }

            //this will put the time series of this node in the file_buffer->buffered_list aray
            //it will include the time series in disk and those in memory

            if (!node->split_node(this, child_node_points, num_child_node_points)) {
                fprintf(stderr, "Error in Index.cpp: could not split node %i | %s.\n", node->id,node->filename);
                return FAILURE;
            }

            free(child_node_points);

            node->file_buffer->do_not_flush = true;

            if (!node->getOrInitFileBuffer(this)) {
                cerr << "Error in Index.cpp:  Could not get the file buffer for node "<<
                     node->id <<" child of node "<<node->parent->id<<endl;

                return FAILURE;
            }

            /*cerr << nodef->node_size << endl;
            exit(-1);*/

            ts_type **ts_list = node->getTS(this);

            //node->leafToBuffer(this);
            //ts_list = node->file_buffer->buffered_list;

            //copying the contents of the the node being split
            //in case it gets flushed from memory to disk

            cout <<"[SPLITTING] Leaf Node "<< node->id
                 <<", of node size "<< node->node_size << ", and disk size "<<node->file_buffer->disk_count
            << ", will be splitted into 2 Leaf node!"<<endl;


            for (int idx = 0; idx < this->index_setting->max_leaf_size; ++idx) {
                if (node->node_split_policy_route_to_left(ts_list[idx])) {

                    node->left_child->updateStatistics(ts_list[idx]);
                    node->left_child->addTS(this,ts_list[idx], true);

                } else {

                    node->right_child->updateStatistics(ts_list[idx]);
                    node->right_child->addTS(this,ts_list[idx], true);
                }
            }
/*
            this->nodes.push(node->left_child);
            this->nodes.push(node->right_child);
*/

            for (int i = 0; i < this->index_setting->max_leaf_size; free(ts_list[++i]));


            cout <<"[SPLITTING SUCCESS] Node "<<node->id<<" has been splitted into 2 new leaves of size "<<node->left_child->node_size
            <<" and "<<node->right_child->node_size<<endl;

            free(ts_list);
            if(!node->deleteFileBuffer(this)){
                cerr <<"Error in hercules_index.c: could not delete file buffer for node "<< node->filename<<endl;
                return FAILURE;
            };

        }

    }
    return SUCCESS;
}
/**
 <center><h3>Add Node's File Buffer to the file buffer map, if the map doesnt exist we init it</h3></center>
 */
bool Index::addFileBuffer(Node *node) const {


    unsigned int idx = this->buffer_manager->file_map_size;

    if (idx == 0) {
        this->buffer_manager->file_map = static_cast<hercules_file_map *>(malloc(sizeof( hercules_file_map)));
        if (this->buffer_manager->file_map == nullptr) {
            fprintf(stderr, "Error in Index.c: Could not allocate memory for the file map.\n");
            return FAILURE;
        }
        this->buffer_manager->file_map[idx].file_buffer = node->file_buffer;
        node->file_buffer->position_in_map = &this->buffer_manager->file_map[idx];

        //index->buffer_manager->file_map[idx].filename = node->filename;
        this->buffer_manager->file_map[idx].prev = nullptr;
        this->buffer_manager->file_map[idx].next = nullptr;

        this->buffer_manager->file_map_tail = this->buffer_manager->file_map;

    } else {
        struct hercules_file_map *lastP = this->buffer_manager->file_map_tail;
        lastP->next = static_cast<hercules_file_map *>(malloc(sizeof(hercules_file_map)));

        if (lastP->next == nullptr) {
            fprintf(stderr, "Error in Index.c: Could not allocate memory for new entry in the Buffer files map.\n");
            return FAILURE;
        }

        lastP->next->file_buffer = node->file_buffer;
        node->file_buffer->position_in_map = lastP->next;

        lastP->next->prev = lastP;
        lastP->next->next = nullptr;
        this->buffer_manager->file_map_tail = lastP->next;

    }

    ++this->buffer_manager->file_map_size;

    return SUCCESS;
}

char * Index::getIndexFilename() const{
    char *filename = static_cast<char *>(malloc(sizeof(char)
            * (strlen(this->index_setting->index_path_hnsw) + 9)));
    filename = strcpy(filename, this->index_setting->index_path_hnsw);
    filename = strcat(filename, "root.idx\0");
    return filename;
}
void Index::write() {
    cout<< "[Storing Index] Store index in "<< this->index_setting->index_path_hnsw<<endl;
    char* filename = this->getIndexFilename();



    FILE *file = fopen(filename, "wb");
    free(filename);

    if (file == nullptr) {
        cerr <<"Error in Index.cpp: Could not open"
                        " the index file. Reason = "<< strerror(errno)<<endl;
        exit(-1);
    }

    unsigned int timeseries_size = this->index_setting->timeseries_size;
    unsigned int max_leaf_size = this->index_setting->max_leaf_size;
    unsigned int init_segments = this->index_setting->init_segments;
    double buffered_memory_size = this->index_setting->buffered_memory_size;

    // SETTINGS DATA

    fwrite(&Node::num_leaf_node, sizeof(unsigned long), 1, file);
    fwrite(&buffered_memory_size, sizeof(double), 1, file);
    fwrite(&timeseries_size, sizeof(unsigned int), 1, file);
    fwrite(&init_segments, sizeof(unsigned int), 1, file);
    fwrite(&max_leaf_size, sizeof(unsigned int), 1, file);


    Node::num_leaf_node = 0;

    // NODES AND FILE BUFFERS
    this->first_node->write(this,file);

    fclose(file);

    cout << "[Index Storing Finished]" << endl;
}

Index *Index::Read(char *path, unsigned int mode) {
    return new Index(path, mode);
}
Index::Index(char *root_directory, unsigned int mode) {
    this->in_memory = mode-1;
    this->time_stats = new timelapse ;
    this->time_stats->index_building_time = 0;
    this->time_stats->querying_time = 0;
    if (chdir(root_directory) != 0 ) {
        fprintf(stderr, "The index directory does not exist. "
                        "Please provide a valid directory.\n");
        exit(-1);
    }
    cout<< "[Loading Index] path : " << root_directory << endl;
    char *index_full_filename = (char *) malloc_index(sizeof(char) * (strlen(root_directory) + 9));
    index_full_filename = strcpy(index_full_filename, root_directory);
    index_full_filename = strcat(index_full_filename, "root.idx\0");
cerr << index_full_filename << endl;
    FILE *file = fopen(index_full_filename, "rb");

    free(index_full_filename);
    unsigned long count_leaves = 0;
    unsigned int timeseries_size = 0;
    unsigned int max_leaf_size = 0;
    unsigned int init_segments = 0;
    double buffered_memory_size = 0;



    fread(&count_leaves, sizeof(unsigned long), 1, file);
    fread(&buffered_memory_size, sizeof(double), 1, file);
    fread(&timeseries_size, sizeof(unsigned int), 1, file);
    fread(&init_segments, sizeof(unsigned int), 1, file);
    fread(&max_leaf_size, sizeof(unsigned int), 1, file);


    this -> index_setting = new Setting(root_directory, root_directory, timeseries_size, init_segments,
                                        max_leaf_size, buffered_memory_size, -1, -1);


    this -> buffer_manager = nullptr;//we dont use mem_array while querying

//    if(in_memory > 1)Node::leaves = static_cast<Node **>(malloc_index(sizeof(Node *) * count_leaves));

    this -> l2space = new hnswlib::L2Space(index_setting->timeseries_size);
    this -> first_node = Node::Read(this, file);


    if(count_leaves != Node::num_leaf_node){
        cerr << "[Bug]Number of expected leaves : "<<count_leaves
        <<", Number of loaded leaves : "<<Node::num_leaf_node<<endl;
        exit(1);
    }

    fclose(file);

    fprintf(stdout, "[Index Loaded Successfully] from: %s "
                    "++++++++ Number of Leaf Node : %lu "
                    "++++++++ NUmber of Internal Node : %lu \n", root_directory, Node::num_leaf_node, Node::num_internal_node);


}
Index::~Index(){
    delete this->time_stats;delete index_setting;
    delete first_node;
}

Node **Index::getLeaves() {
    Node ** leaves = static_cast<Node **>(malloc(sizeof(Node *) * this->first_node->num_leaf_node));
    int i =0;
    first_node->getLeaves(leaves,i);
    return leaves;
}



/**
 <center>
 <b>copy append node buffered_list to disk file, <br>
 and increment the disk_count for each ts. <br>
 turn node.in_disk to true and clear filebuffer</b></center

 * */

