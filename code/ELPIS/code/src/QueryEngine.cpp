//
// Created by zeraph on 25‏/1‏/2021.
//

#include "QueryEngine.h"
QueryEngine::~QueryEngine() {
    free(pq);
    free(results);
    delete flags;
    if(parallel and nprobes > 1){
        for(int i=1;i<nworker;i++){
            free(qwdata[i].stats);
            delete qwdata[i].top_candidates;
            free(qwdata[i].localknn);
            free(qwdata[i].flags);
        }
        free(qwdata);
    }

}
QueryEngine::QueryEngine(const char *query_filename, Index *index, int ef, unsigned int nprobes, bool parallel,
                         unsigned int nworker,bool flatt ,int k) {
    this->query_filename = query_filename;
    this->index = index;
    this->index->ef = ef;
    this->nprobes = nprobes;
    this->parallel = parallel;
    this->nworker = nworker;
    this->results = static_cast<float *>(malloc(sizeof(float) * k));

    this->flags = new hnswlib::vl_type [Node::max_leaf_size];
    memset(this->flags, 0, sizeof(hnswlib::vl_type) * Node::max_leaf_size);
    this->curr_flag = 0;//max value of ushort is 65K => in total of search on all queries, we should not exceed 65k searchleaf

    this->pq = pqueue_init(Node::num_leaf_node,
                           cmp_pri, get_pri, set_pri, get_pos, set_pos);


    //sometimes std::thread return 0 so, we use sysconf(LINUX)
    unsigned short local_nprobes = 0;

    if(parallel and nprobes >1){
        this->candidate_leaves =  pqueue_init(nprobes-1,
                                              cmp_pri, get_pri, set_pri, get_pos, set_pos);


        this->nworker = (std::thread::hardware_concurrency()==0)? sysconf(_SC_NPROCESSORS_ONLN) : std::thread::hardware_concurrency() -1;
        if(this->nworker>nprobes-1)this->nworker = nprobes-1;//to ensure that in balanced load no worker will remain idle
        this->qwdata = static_cast<worker_backpack__ *>(malloc(sizeof(worker_backpack__) * this->nworker));

        local_nprobes = (nprobes-1) / this->nworker;
        for(int i=1;i<this->nworker;i++){
            qwdata[i].stats = static_cast<querying_stats *>(malloc(sizeof(querying_stats)));
            qwdata[i].top_candidates = new std::priority_queue<std::pair<float, unsigned int>, std::vector<std::pair<float, unsigned int>>>();
            qwdata[i].local_nprobes = (local_nprobes);
            qwdata[i].localknn = static_cast<float*>(malloc(k*sizeof(float)));
            qwdata[i].flags = new hnswlib::vl_type [Node::max_leaf_size];
            memset(qwdata[i].flags, 0, sizeof(hnswlib::vl_type) * Node::max_leaf_size);
            qwdata[i].curr_flag = 0;


        }
//        qwdata[0].localknn = static_cast<float *>(malloc(k*sizeof(float)));
        qwdata[0].local_nprobes += (nprobes-1)%this->nworker;
        qwdata[0].stats = &stats;
        qwdata[0].top_candidates = &top_candidates;

        omp_set_dynamic(0);
        omp_set_num_threads(this->nworker);

    }
    setEF(index->first_node,ef);
    if(parallel){
        std::cout << "[QUERYING PARAM]  EF : " <<this->index->ef
                  <<"| NPROBES : " <<nprobes
                  <<"| PARALLEL : "<<this->parallel
                  <<"| NWORKERS : " <<this->nworker
                  <<"| Nprobes/Worker :"<<local_nprobes
                  <<std::endl;
    }
    else{
        std::cout << "[QUERYING PARAM]  EF : " <<this->index->ef
                  <<"| NPROBES : " <<nprobes
                  <<"| PARALLEL : "<<0
                  <<std::endl;
    }
}


void QueryEngine::setEF(Node * node, int ef){
    if(node->is_leaf)node->leafgraph->setEf(ef);
    else{
        setEF(node->left_child,ef);
        setEF(node->right_child,ef);
    }
}
void QueryEngine::queryBinaryFile(int q_num, unsigned int k, int mode) {
    // Record start time
    auto start = now();

    this->query_file = fopen(this->query_filename, "rb");
    if (this->query_file  == NULL) {
        fprintf(stderr, "Queries file %s not found!\n", this->query_filename);
        exit(-1);
    }

    fseek(this->query_file, 0L, SEEK_END);
    file_position_type sz = (file_position_type) ftell(this->query_file);
    fseek(this->query_file, 0L, SEEK_SET);
    this->total_records = sz / this->index->index_setting->timeseries_size * sizeof(ts_type);


    fseek(this->query_file, 0L, SEEK_SET);
    unsigned int offset = 0;

    if (this->total_records < q_num) {
        fprintf(stderr, "File %s has only %llu records!\n", query_filename, total_records);
        exit(-1);
    }

    unsigned int q_loaded = 0;
    unsigned int ts_length = this->index->index_setting->timeseries_size;

    ts_type *query_ts = static_cast<ts_type *>(malloc_search( sizeof(ts_type) * ts_length));
    cout << query_filename<<endl;

     while(q_loaded < q_num){
        q_loaded++;
        fread(query_ts, sizeof(ts_type), ts_length, this->query_file);
        searchNpLeafParallel(query_ts,k,nprobes);
    }

    free(query_ts);

    this->closeFile();



    index->time_stats->querying_time = getElapsedTime(start);

}


void QueryEngine::closeFile(){
    if (fclose(this->query_file)) {
        fprintf(stderr, "Error: Could not close the query filename %s", query_filename);
        exit(111);
    }
    printf( "[Query File Closed] %s \n", query_filename);
}





void searchflat(Node * node, unsigned int entrypoint, const void *data_point, size_t beamwidth,size_t k,
                std::priority_queue<std::pair<float,unsigned int>, std::vector<std::pair<float,unsigned int>>> & top_candidates,
                float & bsf,querying_stats & stats, unsigned short *threadvisits, unsigned short & round_visit) {
    auto g = node->leafgraph;
    round_visit++;
    std::priority_queue<std::pair<float, unsigned int>, std::vector<std::pair<float, unsigned int>>, CompareByFirst> candidate_set;

    float LBGRAPH;

    float dist = g->fstdistfunc_(data_point, g->getDataByInternalId(entrypoint), g->dist_func_param_);
#ifdef CALC_DC
    stats.distance_computations_bsl++;
#endif
    LBGRAPH = dist;
    top_candidates.emplace(dist, entrypoint);
    candidate_set.emplace(-dist, entrypoint);
    threadvisits[round_visit] = round_visit;

    while (candidate_set.size() > 0) {

        auto currnode = candidate_set.top();

        if ((-currnode.first) > LBGRAPH) {
            break;
        }
        candidate_set.pop();

        unsigned int currnodeid = currnode.second;
        int *data = (int *) g->get_linklist0(currnodeid);
        size_t neighborhood = g->getListCount((unsigned int*)data);


        _mm_prefetch((char *) (threadvisits + *(data + 1)), _MM_HINT_T0);
        _mm_prefetch((char *) (threadvisits + *(data + 1) + 64), _MM_HINT_T0);
        _mm_prefetch(g->data_level0_memory_ + (*(data + 1)) * g->size_data_per_element_ + g->offsetData_, _MM_HINT_T0);
        _mm_prefetch((char *) (data + 2), _MM_HINT_T0);

        for (size_t j = 1; j <= neighborhood; j++) {
            int neighborid = *(data + j);

            _mm_prefetch((char *) (threadvisits + *(data + j + 1)), _MM_HINT_T0);
            _mm_prefetch(g->data_level0_memory_ + (*(data + j + 1)) * g->size_data_per_element_ + g->offsetData_,
                         _MM_HINT_T0);
            if (threadvisits[neighborid] == round_visit) continue;


            threadvisits[neighborid] = round_visit;

            char *currObj1 = (g->getDataByInternalId(neighborid));
            auto dist = g->fstdistfunc_(data_point, currObj1, g->dist_func_param_);
#ifdef CALC_DC
                stats.distance_computations_bsl++;
#endif
            if(dist < bsf)bsf=dist;

            if (top_candidates.size() < beamwidth|| LBGRAPH > dist) {
                    candidate_set.emplace(-dist, neighborid);

                    _mm_prefetch(g->data_level0_memory_ + candidate_set.top().second * g->size_data_per_element_ +
                                 g->offsetLevel0_,///////////
                                 _MM_HINT_T0);////////////////////////


                    top_candidates.emplace(dist, neighborid);

                    if (top_candidates.size() > beamwidth)
                        top_candidates.pop();

                    if (!top_candidates.empty())
                        LBGRAPH = top_candidates.top().first;

            }
            }
        }
    }



void searchGraphLeaf(Node * node,const void *query_data, size_t k,
                     std::priority_queue<std::pair<float, unsigned int>, std::vector<std::pair<float, unsigned int>>> &top_candidates,
                     float &bsf, querying_stats &stats, unsigned short *flags, unsigned short & flag)  {
    auto g = node->leafgraph;
    unsigned int currObj = g->enterpoint_node_;
    float curdist = g->fstdistfunc_(query_data, g->getDataByInternalId(currObj), g->dist_func_param_);
    for (int level = g->maxlevel_; level > 0; level--) {
        bool changed = true;
        while (changed) {
            changed = false;
            unsigned int *data;
            data = (unsigned int *) g->get_linklist(currObj, level);
            int size = g->getListCount(data);
            unsigned int *datal = (unsigned int *) (data + 1);
            for (int i = 0; i < size; i++) {
                unsigned int cand = datal[i];
                if (cand < 0 || cand > g->max_elements_)
                    throw std::runtime_error("cand error");
#ifdef CALC_DC
                stats.distance_computations_hrl++;
#endif
                float d = g->fstdistfunc_(query_data, g->getDataByInternalId(cand), g->dist_func_param_);

                if (d < curdist) {
                    curdist = d;
                    currObj = cand;
                    changed = true;
                }
            }
        }
    }

    searchflat(node, currObj, query_data, std::max(g->ef_, k), k , top_candidates, bsf, stats,
               flags, flag);
    while (top_candidates.size() > k)top_candidates.pop();

};




void QueryEngine::searchNpLeafParallel(ts_type *query_ts, unsigned int k, unsigned int nprobes) {

    stats.reset();

    Time start = now();

    ts_type kth_bsf = FLT_MAX;

    Node *App_node = this->index->first_node;
    ts_type App_bsf;
    if (App_node == nullptr) throw std::runtime_error("Error : First node == nullptr!");
    while (!App_node->is_leaf) {
        if (App_node->node_split_policy_route_to_left(query_ts)) {
            App_node = App_node->left_child;
        } else {
            App_node = App_node->right_child;
        }
    }



    searchGraphLeaf(App_node,query_ts, k,
                    top_candidates,App_bsf, stats, flags,
                    curr_flag);



    nprobes--;
    if (nprobes == 0) {
        while (top_candidates.size() > k)top_candidates.pop();

        while (top_candidates.size() > 0) {
            results[top_candidates.size() - 1] = top_candidates.top().first;
            top_candidates.pop();
        }
        double time = getElapsedTime(start);
        printKNN(results, k, time, visited, 0);

    }
    else{
        auto *root_pq_item = static_cast<query_result *>(malloc_search(sizeof(struct query_result)));

        root_pq_item->node = this->index->first_node;
        root_pq_item->distance = this->index->first_node->calculate_node_min_distance(this->index, query_ts, stats);

        pqueue_insert(pq, root_pq_item);

        struct query_result *n;
        ts_type child_distance;
        ts_type bsf = FLT_MAX;


        query_result * candidates =  static_cast<query_result *>(calloc(Node::num_leaf_node+1, sizeof(struct query_result)));
        unsigned int candidates_count = 0;
        int pos;
        kth_bsf = top_candidates.top().first;
        while ((n = static_cast<query_result *>(pqueue_pop(pq)))) {
            if (n->distance > kth_bsf) {//getting through two pruning process is tricky...
                break;
            }
            if (n->node->is_leaf) // n is a leaf
            {

                pos = candidates_count - 1;

                if (pos >= 0)
                    while (pos >= 0 and n->distance < candidates[pos].distance) {
                        candidates[pos + 1].node = candidates[pos].node;
                        candidates[pos + 1].distance = candidates[pos].distance;
                        pos--;
                    }
                candidates[pos + 1].node = n->node;
                candidates[pos + 1].distance = n->distance;
                candidates_count++;
            } else
            {
                child_distance = n->node->left_child->calculate_node_min_distance(this->index, query_ts, stats);
                if ((child_distance < kth_bsf) &&
                    (n->node->left_child != App_node)) //add epsilon
                {
                    auto *mindist_result_left = static_cast<query_result *>(malloc_search(
                            sizeof(struct query_result)));
                    mindist_result_left->node = n->node->left_child;
                    mindist_result_left->distance = child_distance;
                    pqueue_insert(pq, mindist_result_left);
                }

                child_distance = n->node->right_child->calculate_node_min_distance(this->index, query_ts, stats);
                if ((child_distance < kth_bsf) &&
                    (n->node->right_child != App_node)) //add epsilon
                {
                    auto *mindist_result_right = static_cast<query_result *>(malloc_search(
                            sizeof(struct query_result)));
                    mindist_result_right->node = n->node->right_child;
                    mindist_result_right->distance = child_distance;
                    pqueue_insert(pq, mindist_result_right);
                }
            }

            free(n);
        }
        stats.num_candidates = candidates_count;

        for (int i = 1; i < nworker; i++) {
            qwdata[i].id = i;
            qwdata[i].kth_bsf = &kth_bsf;
            qwdata[i].stats->reset();
            qwdata[i].bsf = FLT_MAX;
        }
        qwdata[0].kth_bsf = &kth_bsf;
        qwdata[0].bsf = FLT_MAX;
        qwdata[0].id=0;
        qwdata[0].flags = flags;
        qwdata[0].curr_flag = curr_flag;

        copypq(qwdata, top_candidates);



        pthread_rwlock_t lock_bsf = PTHREAD_RWLOCK_INITIALIZER;

        query_result node;
        query_worker_data *worker;

        {
#pragma omp parallel num_threads(nworker) private(node, bsf, worker) shared(qwdata, candidates_count, candidates,  query_ts, k)
            {
                bsf = FLT_MAX;
                worker = qwdata+omp_get_thread_num();
#pragma omp for schedule(static, 1)
                for (int i = 0; i < std::min(candidates_count,nprobes); i++) {

                    node = candidates[i];
                    pthread_rwlock_rdlock(&lock_bsf);
                    bsf = *worker->kth_bsf;
                    pthread_rwlock_unlock(&lock_bsf);
                    worker->stats->num_leaf_checked++;
                    //                    worker->checked_leaf.push(node.node->id);
                    if (node.distance <=worker->bsf) {
                        worker->stats->num_leaf_searched++;

                        searchGraphLeaf(node.node,query_ts, k,
                                        *(worker->top_candidates),worker->bsf, *(worker->stats), worker->flags,
                                        worker->curr_flag);

                        if (worker->top_candidates->top().first < bsf) {
                            pthread_rwlock_wrlock(&lock_bsf);
                            *(worker->kth_bsf) = worker->top_candidates->top().first;
                            pthread_rwlock_unlock(&lock_bsf);
                        }

                    }
                }
#pragma omp for schedule(static, 1)
                for(int i=1;i<nworker;i++){
                    if(qwdata[i].stats->num_leaf_searched==0)while(qwdata[i].top_candidates->size()>0)qwdata[i].top_candidates->pop();

                    else {

                        for (int j = k - 1; j >= 0; j--) {

                            if(qwdata[i].top_candidates->top().second ==this->index->index_setting->max_leaf_size + 1)
                                qwdata[i].localknn[j] = FLT_MAX;
                            else{qwdata[i].localknn[j] = qwdata[i].top_candidates->top().first;}

                            qwdata[i].top_candidates->pop();
                        }
                    }
                }
            }
        }


        for(int i=1;i<nworker;i++){
            if(qwdata[i].stats->num_leaf_searched==0)continue;
            for(int j = 0;j<k;j++){
                if(qwdata[i].localknn[j]==FLT_MAX)continue;
                if(qwdata[i].localknn[j] < top_candidates.top().first) {
                    top_candidates.emplace(qwdata[i].localknn[j], 0);
                    top_candidates.pop();
                } else {
                    break;
                }
            }
        }


        while(top_candidates.size()>k)top_candidates.pop();
        while (top_candidates.size() > 0) {
            results[top_candidates.size() - 1] = top_candidates.top().first;
            top_candidates.pop();
        }

        double time = getElapsedTime(start);


        for (int i = 1; i < nworker; i++) {

//            cout << "worker "<<i<<": visited "<<qwdata[i].stats->num_leaf_searched<<endl;
            stats.num_knn_alters += qwdata[i].stats->num_knn_alters;
            stats.num_leaf_checked += qwdata[i].stats->num_leaf_checked;
            stats.num_leaf_searched += qwdata[i].stats->num_leaf_searched;
            stats.distance_computations_hrl += qwdata[i].stats->distance_computations_hrl;
            stats.distance_computations_bsl += qwdata[i].stats->distance_computations_bsl;

        }


        printKNN(results, k, time, visited, 1);



        // Free the nodes that were not popped.
        while ((n = static_cast<query_result *>(pqueue_pop(pq))))free(n);
        while ((n = static_cast<query_result *>(pqueue_pop(candidate_leaves))))free(n);

    }

}


inline void QueryEngine::printKNN(float * results, int k, double time,queue<unsigned int> & visited, bool para){
    cout << "----------"<<k<<"-NN RESULTS----------- | "<<para;
    if(para)cout<<" - num candidates "<<stats.num_candidates
                << " - num leaf checked "<<stats.num_leaf_checked
                << " - num leaf searched "<<stats.num_leaf_searched
                << " - num update kth_bsf "<<stats.num_knn_alters;
    cout<<" | visited nodes : ";
    for(;!visited.empty();visited.pop())cout<< visited.front() << " ";
    cout << endl;
    for(int i = 0 ; i < k ; i++){
        printf( " K N°%i  => Distance : %f | Node ID : %lu | Time  : %f |Total DC : %lu | HDC : %lu | BDC : %lu \n",i+1,sqrt(results[i]),
                0,time,stats.distance_computations_hrl+stats.distance_computations_bsl,stats.distance_computations_hrl,stats.distance_computations_bsl);
        stats.reset();
        /*    cout << " K N°"<<i+1<<" => Distance : "<<sqrt(results[i])
                 << " | Node ID : "<< 0
                 << " | Time : "<<time
                 << " | Time(calcul_node_min_distance): "<<stats.time_cnmd
                 << " | Time(update knn): "<<stats.time_update_knn
                 << " | Time(leaves searchknn): "<<stats.time_leaves_search
                 << " | Time(leaves searchknn@routing): "<<stats.time_routing
                 << " | Time(leaves searchknn@basedlayer): "<<stats.time_layer0
                 << " | Time(leaves searchknn@pq): "<<stats.time_pq
                 << " | Time(sum): " << stats.time_layer0+stats.time_routing+stats.time_pq
                 //             << " | Node Level : "<< knn_results[i].node->level
                 << " | Num DC(hrl): "<< stats.distance_computations_hrl
                 << " | Num Hops(hrl): "<< stats.num_hops_hrl
                 << " | Num DC(bsl): "<< stats.distance_computations_bsl
                 << " | Num Hops(bsl): "<< stats.num_hops_bsl
                 << " | Num DC(lb): " << stats.distance_computations_lb*/
        time = 0;
    }
}

inline void QueryEngine::copypq(query_worker_data *pData, priority_queue<pair<float, unsigned int>> queue) {
    while(!queue.empty()){
        auto p = queue.top();
        p.second  =  this->index->index_setting->max_leaf_size + 1;
        for(int i =1; i<nworker;i++){//0 is reserved to top_candidates itself, AND worker 0 has a reference to PQ
            pData[i].top_candidates->emplace(p);
        }
        queue.pop();
    }
}

