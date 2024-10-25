#include <iostream>
#include "include/hnswlib/hnswlib.h"
#include <getopt.h>

using namespace hnswlib;
using namespace std;

typedef float ts_type;


#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <time.h>

#include "sys/stat.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <getopt.h>
#include <cstdlib>
#include <dirent.h>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>
#include "hnswlib/hnswlib.h"
#include "dirent.h"

#include <unordered_set>

using namespace std;
using namespace hnswlib;

void
query_workload(size_t vecsize,
               size_t qsize, HierarchicalNSW<ts_type> &appr_alg,
               size_t vecdim, vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
               size_t k, char * queries, size_t efs,bool flatt,int sims);
void
query_workload_IQP(size_t vecsize,
               size_t qsize, HierarchicalNSW<ts_type> &appr_alg,
               size_t vecdim, vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
               size_t k, char * queries, size_t efs,bool flatt,int sims);

void printKNN(float * results, int k,  querying_stats stats, unsigned int *ids = nullptr);


void read_data(char * dataset,
               ts_type ** pdata,
               unsigned int ts_length,
               unsigned int data_size,
               unsigned int offset);

void add_data(HierarchicalNSW<ts_type> &appr_alg, ts_type *data, unsigned int ts_length, unsigned int data_size,
              unsigned int label_offset, int i, float d, int cnt, int range=0);
void add_data_ksrep(HierarchicalNSW<ts_type> &appr_alg, ts_type *data, unsigned int ts_length, unsigned int data_size,
               unsigned int label_offset, int i, float d);
void query_workloadrdseed(        size_t vecsize,        size_t qsize,        HierarchicalNSW<ts_type> &appr_alg,        size_t vecdim,        vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
        size_t k,        char * queries,        size_t efs);



void query_workload_kdt(size_t vecsize,
                             size_t qsize,
                             HierarchicalNSW<ts_type> &appr_alg,
                             size_t vecdim,
                             vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
                             size_t k,
                             char * queries,
                             size_t efs, uint **kdeps);

void peak_memory_footprint() {

    unsigned iPid = (unsigned)getpid();

    std::cout<<"PID: "<<iPid<<std::endl;

    std::string status_file = "/proc/" + std::to_string(iPid) + "/status";
    std::ifstream info(status_file);
    if (!info.is_open()) {
        std::cout << "memory information open error!" << std::endl;
    }
    std::string tmp;
    while(getline(info, tmp)) {
        if (tmp.find("Name:") != std::string::npos || tmp.find("VmPeak:") != std::string::npos || tmp.find("VmHWM:") != std::string::npos)
            std::cout << tmp << std::endl;
    }
    info.close();

}

#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <omp.h>
#include <random>
#include <algorithm>
#include <unordered_set>
using namespace std;

int findApproxDiameter(HierarchicalNSW<ts_type> &graph, int num_samples = 1000000) {
    int diameter = 0;
    int n = graph.cur_element_count;

    random_device rd;
    mt19937 gen(1996); // Setting the seed to 1996
    uniform_int_distribution<unsigned int> dist(0, n - 1);

    auto bfs = [&](int start) {
        vector<bool> visited(n, false);
        queue<pair<int, int>> q;
        q.push({start, 0});
        visited[start] = true;
        int max_distance = 0;

        while (!q.empty()) {
            auto [node, distance] = q.front();
            q.pop();
            if (distance > max_distance) {
                max_distance = distance;
            }
            auto data = graph.get_linklist0(node);
            auto size = graph.getListCount(data);

            for (int j = 1; j <= size; ++j) {
                unsigned neighbor = data[j];
                if (!visited[neighbor]) {
                    q.push({neighbor, distance + 1});
                    visited[neighbor] = true;
                }
            }
        }
        return max_distance;
    };

    unordered_set<unsigned int> sampled_nodes;
    while (sampled_nodes.size() < num_samples) {
        sampled_nodes.insert(dist(gen));
    }
    vector<unsigned int> sampled_nodes_vec(sampled_nodes.begin(), sampled_nodes.end());

#pragma omp parallel for reduction(max:diameter)
    for (size_t i = 0; i < sampled_nodes_vec.size(); ++i) {
        int node = sampled_nodes_vec[i];
        int current_diameter = bfs(node);
#pragma omp critical
        {
            diameter = max(diameter, current_diameter);
        }
    }

    return diameter;
}

int findDiameter(HierarchicalNSW<ts_type> &graph) {
    int diameter = 0;
    int n = graph.cur_element_count;


    auto bfs = [&](int start) {
        vector<bool> visited(n, false);
        queue<pair<int, int>> q;
        q.push({start, 0});
        visited[start] = true;
        int maxDistance = 0;

        while (!q.empty()) {
            auto [node, distance] = q.front();
            q.pop();
            if (distance > maxDistance) {
                maxDistance = distance;
            }
            auto data = graph.get_linklist0(node);
            auto size = graph.getListCount(data);

            for (int j =1;j<=size;j++) {
                unsigned neighbor = data[j];
                if (!visited[neighbor]) {
                    q.push({neighbor, distance + 1});
                    visited[neighbor] = true;
                }
            }
        }
        return maxDistance;
    };

#pragma omp parallel for reduction(max:diameter)
    for (int i = 0; i < n; ++i) {
        int currentDiameter = bfs(i);
        if (currentDiameter > diameter) {
#pragma omp critical
            {
                if (currentDiameter > diameter) {
                    diameter = currentDiameter;
                }
            }
        }
    }

    return diameter;
}

int main (int argc, char **argv) {
    static char *dataset = "/data_current.txt";
    static char *queries = "/query_current.txt";

    static char *index_path = "out/";
    static unsigned int dataset_size = 1000;
    static unsigned int queries_size = 5;
    static unsigned int ts_length = 256;
    static unsigned int init_segments = 1;
    static unsigned int leaf_size = 100;
    static int use_ascii_input = 0;
    static int mode = 0;
    static int efs = 0; //perform exact search by default
    int k = 1;
    int efConstruction = 500;
    int M = 4;
    int ep = 0;
    int rng = 0; // type of nd : RND RRND MOND NOND
    float prune = 1;
    int connectivity = 1;
    int depth = 8;
    int ntrees = 8;
    int range = 0;//use visited listed instead of top K nn as candidate neighbors set
    while (1) {
        static struct option long_options[] = {
                {"efconstruction",  required_argument, 0, 'b'},
                {"dataset",         required_argument, 0, 'd'},
                {"ef",              required_argument, 0, 'e'},
                {"k",               required_argument, 0, 'k'},
                {"queries",         required_argument, 0, 'q'},
                {"index-path",      required_argument, 0, 'p'},
                {"dataset-size",    required_argument, 0, 'z'},
                {"queries-size",    required_argument, 0, 'g'},
                {"mode",            required_argument, 0, 'x'},
                {"timeseries-size", required_argument, 0, 't'},
                {"M",               required_argument, 0, 'm'},
                {"ep",required_argument, 0, 'ep'},
                {"rng",required_argument, 0, 'rn'},
                {"prune",required_argument, 0, 'pr'},
                {"cnt",required_argument, 0, 'c'},
                {"depth",required_argument, 0, 'dp'},
                {"nt",required_argument, 0, 'nt'},
                  {"range", required_argument, 0, 'lr'},

                {"help",            no_argument,       0, '?'}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "",
                            long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'lr':
                range =atoi(optarg);
                break;
            case 'pr':
                prune =atof(optarg);
                break;
            case 'rn':
                rng =atoi(optarg);
                break;
            case 'ep':
                ep =atoi(optarg);
                break;
            case 'q':
                queries = optarg;
                break;
            case 'b':
                efConstruction = atoi(optarg);
                break;
            case 'e':
                efs = atoi(optarg);
                break;
            case 'k':
                k = atoi(optarg);
                if (k < 1) {
                    fprintf(stderr, "Please change k to be greater than 1.\n");
                    exit(-1);
                }
                break;
            case 'g':
                queries_size = atoi(optarg);
                if (queries_size < 1) {
                    fprintf(stderr, "Please change the queries size to be greater than 0.\n");
                    exit(-1);
                }
                break;
            case 'd':
                dataset = optarg;
                break;
            case 'p':
                index_path = optarg;
                break;
            case 'dp':
                depth = atoi(optarg);
                break;
            case 'nt':
                ntrees = atoi(optarg);
                break;
            case 'x':
                mode = atoi(optarg);
                break;

            case 'z':
                dataset_size = atoi(optarg);
                if (dataset_size < 1) {
                    fprintf(stderr, "Please change the dataset size to be greater than 0.\n");
                    exit(-1);
                }
                break;
            case 'c':
                connectivity = atoi(optarg);
                break;
            case 't':
                ts_length = atoi(optarg);
                break;

            case 'm':
                M = atoi(optarg);
                break;
            default:
                exit(-1);
                break;
        }
    }

    L2Space l2space(ts_length);

    char *index_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 10));
    index_full_filename = strcpy(index_full_filename, index_path);
    index_full_filename = strcat(index_full_filename, "index.bin");

    if (mode == 0)  //only build and store the index
    {

        if(chdir(index_path) == 0)
            throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");
        mkdir(index_path , 07777);


        ts_type *data = (ts_type *)malloc(dataset_size * ts_length * sizeof(ts_type));

        HierarchicalNSW<ts_type> appr_alg(&l2space, dataset_size, M, efConstruction);

        auto t_build = new PTK::Timer() ;

        int i;

        auto buffer_size = 100000;
        if(buffer_size > dataset_size)buffer_size=dataset_size;
        unsigned int chunk_count = (unsigned  int) (dataset_size / buffer_size);
        unsigned int last_chunk_size = dataset_size % buffer_size;
        unsigned int chunk_size = buffer_size;

        double kPi = 3.14159265358979323846264;
        if(rng == 2) {
            cout << "angle "<< prune ;
            prune = std::cos(prune / 180 * kPi);
            cout << " cos "<<prune << endl;
        }

    for (i = 0; i < chunk_count; ++i) {
        printf("Loading %ld vectors of chunk %ld\n", chunk_size, i + 1);
        read_data(dataset, &data, ts_length, chunk_size, i * chunk_size);
        if(ep==0)
        add_data(appr_alg, data, ts_length, chunk_size, i * chunk_size, rng, prune,connectivity, range);
        if(ep==3)
        add_data_ksrep(appr_alg, data, ts_length, chunk_size, i * chunk_size, rng, prune);
    }
    if (last_chunk_size != 0) {
        printf("Loading %ld vectors of the last chunk %ld\n", last_chunk_size, i + 1);
        read_data(dataset, &data, ts_length, last_chunk_size, i * chunk_size);
        if(ep==0)add_data(appr_alg, data, ts_length, last_chunk_size, i * chunk_size, rng, prune,connectivity,range);
        if(ep==3)add_data_ksrep(appr_alg, data, ts_length, last_chunk_size, i * chunk_size, rng, prune);
    }


        t_build->printElapsedTime(std::string ("Index Building").c_str());

        t_build->restart();

#ifdef DC_IDX
        cout << "The number of distance calculations "<<hnswlib::dc_counter.load(std::memory_order_relaxed) << endl;
#endif
        appr_alg.saveIndex(index_full_filename);

        t_build->printElapsedTime(std::string ("Index Saving").c_str());

    }
    else if (mode == 1)  //read an existing index and execute queries
    {
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder doesn't exist, Please make sure to give an existing index path!");

        // HIERARCHY
        if(ep ==0) {
            HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);
            auto s_build = new PTK::Timer();
            vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
            query_workload(
                    (size_t) dataset_size,
                    (size_t) queries_size,
                    appr_alg,
                    (size_t) ts_length,
                    answers,
                    (size_t) k,
                    queries,
                    (size_t) efs, 0, 0);

//            s_build->printElapsedTime(std::string("TOTAL TIME").c_str());
        }
        // MEDOID
            //calculate medoid
        else if(ep == 15){ // save the meoid in index/medoid.bin
            HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);

            auto dim = *((int*)appr_alg.dist_func_param_) ;

            float * centroid = new float[dim] ;
            for(int i =0;i<dim;i++)centroid[i]=0;
            for(int i = 0; i<appr_alg.data_size_;i++){
                auto vec = (float *) appr_alg.getDataByInternalId(i);
                for(int j =0;j<dim;j++)centroid[j]+=vec[j];
            }
            for(int j =0;j<dim;j++)centroid[j]=centroid[j]/appr_alg.data_size_;
            appr_alg.setEf(1000);
            querying_stats s;
            float distg = MAXFLOAT;
            unsigned int idmaxg;
            {
#pragma omp parallel
                {
                    unsigned int idmax;
                    float dist = MAXFLOAT;
#pragma omp for
                    for (int i = 0; i < appr_alg.data_size_; i++) {
                        float tmp = appr_alg.fstdistfunc_(appr_alg.getDataByInternalId(i), centroid,appr_alg.dist_func_param_);
                        if (tmp < dist) {
                            dist = tmp;
                            idmax = i;
                        }
                    }

#pragma omp critical
                    if (distg > dist) {
                        distg = dist;
                        idmaxg = idmax;
                    }
                }
            }

            appr_alg.enterpoint_node_ = idmaxg;
            char *index_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 12));
            index_full_filename = strcpy(index_full_filename, index_path);
            index_full_filename = strcat(index_full_filename, "medoid.bin");
            auto file = fopen(index_full_filename,"wb");
            fwrite(&idmaxg,sizeof(unsigned int),1,file);
            fclose(file);
        }
            // search using medoid as ep
        else if(ep == 1){ // medoid 1
            HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);

            auto dim = *((int*)appr_alg.dist_func_param_) ;


            char *index_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 12));
            index_full_filename = strcpy(index_full_filename, index_path);
            index_full_filename = strcat(index_full_filename, "medoid.bin");
            unsigned int r;
            auto file = fopen(index_full_filename,"rb");
            fread(&r,sizeof(unsigned int),1,file);
            fclose(file);


            appr_alg.enterpoint_node_ = r;
;
            auto s_build = new PTK::Timer();
            vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
            query_workload(
                    (size_t) dataset_size,
                    (size_t) queries_size,
                    appr_alg,
                    (size_t) ts_length,
                    answers,
                    (size_t) k,
                    queries,
                    (size_t) efs, 1, 0);

            s_build->printElapsedTime(std::string("TOTAL TIME").c_str());

        }
        // PREDEFINED 1 RANDOM POINT
        if(ep ==2) {//1 random point
            HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);
            auto s_build = new PTK::Timer();
            vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
            query_workload(
                    (size_t) dataset_size,
                    (size_t) queries_size,
                    appr_alg,
                    (size_t) ts_length,
                    answers,
                    (size_t) k,
                    queries,
                    (size_t) efs, 1, 0);

            s_build->printElapsedTime(std::string("TOTAL TIME").c_str());
        }
        // SAMPLE K init candidate
        if(ep ==3) {
            HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);
            auto s_build = new PTK::Timer();
            vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
            query_workloadrdseed(
                    (size_t) dataset_size,
                    (size_t) queries_size,
                    appr_alg,
                    (size_t) ts_length,
                    answers,
                    (size_t) k,
                    queries,
                    (size_t) efs);

            s_build->printElapsedTime(std::string("TOTAL TIME").c_str());
        }
        if(ep==4){
            HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);
            char *kdtreesrpath = (char *) malloc(sizeof(char) * (strlen(index_path) + 10));
            kdtreesrpath = strcpy(kdtreesrpath, index_path);
            kdtreesrpath = strcat(kdtreesrpath, "kdtrs.bin");
            int maxl = 1024;
            auto kdeps = new uint *[queries_size];
            FILE *file = fopen(kdtreesrpath, "r");
            for(int i =0;i<queries_size;i++){
                kdeps[i] = new uint[maxl];
                fread(kdeps[i], sizeof(uint),maxl,file);
            }
            fclose(file);
            auto s_build = new PTK::Timer();
            vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
            query_workload_kdt(
                    (size_t) dataset_size,
                    (size_t) queries_size,
                    appr_alg,
                    (size_t) ts_length,
                    answers,
                    (size_t) k,
                    queries,
                    (size_t) efs, kdeps);

            s_build->printElapsedTime(std::string("TOTAL TIME").c_str());
        }
    }
else   if(mode == 6){
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder doesn't exist, Please make sure to give an existing index path!");

        HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);
        auto s_build = new PTK::Timer();
        ts_type  * query = (float * ) aligned_alloc(8* sizeof(float),
                                                    ts_length*queries_size* sizeof(float));

        FILE *dfp = fopen(queries, "rb");
        if (dfp  == NULL) {
            fprintf(stderr, "Queries file %s not found!\n",dfp);
            exit(-1);
        }
        fread(query, sizeof(ts_type), ts_length*queries_size, dfp);
        fclose(dfp);
        appr_alg.setEf(efs);
        querying_stats s;
        float* result;

        s.time_cnmd=0;s.time_update_knn=0;s.time_leaves_search=0;s.time_routing=0;s.time_layer0=0;s.time_pq=0;
        s.num_hops_bsl=0;s.distance_computations_hrl=0;s.num_hops_hrl=0;s.distance_computations_bsl=0;
        s.saxdist_computations_bsl=0;s.saxdist_computations_hsl=0;

        for(int i=0;i<queries_size;i++){
            result = appr_alg.searchGraphBSFSPQALIGNMEM(query + i*ts_length, k, s);
            printKNN(result, k, s);
            s.time_cnmd=0;s.time_update_knn=0;s.time_leaves_search=0;s.time_routing=0;s.time_layer0=0;s.time_pq=0;
            s.num_hops_bsl=0;s.distance_computations_hrl=0;s.num_hops_hrl=0;s.distance_computations_bsl=0;
            s.saxdist_computations_bsl=0;s.saxdist_computations_hsl=0;
        };
        s_build->printElapsedTime(std::string("TOTAL TIME").c_str());
    }
    else if(mode == 7){
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder doesn't exist, Please make sure to give an existing index path!");

        HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, 7);

        auto s_build = new PTK::Timer();

        ts_type  * query = (float * ) aligned_alloc(8* sizeof(float),appr_alg.dim_*queries_size* sizeof(float));

        FILE *dfp = fopen(queries, "rb");
        if (dfp  == NULL) {
            fprintf(stderr, "Queries file %s not found!\n",dfp);
            exit(-1);
        }

        fread(query, sizeof(ts_type), appr_alg.dim_*queries_size, dfp);
        fclose(dfp);
        appr_alg.setEf(efs);

        querying_stats s;
        float* result;


        for(int i=0;i<queries_size;i++){
            result = appr_alg.searchGraphBSFSPQ(query + i*appr_alg.dim_, k, s);

            printKNN(result, k, s);

            s.time_cnmd=0;s.time_update_knn=0;s.time_leaves_search=0;s.time_routing=0;s.time_layer0=0;s.time_pq=0;
            s.num_hops_bsl=0;s.distance_computations_hrl=0;s.num_hops_hrl=0;s.distance_computations_bsl=0;
            s.saxdist_computations_bsl=0;s.saxdist_computations_hsl=0;
        };
        s_build->printElapsedTime(std::string("TOTAL TIME").c_str());

    }
    //DUMY
    else if(mode ==4){
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder doesn't exist, Please make sure to give an existing index path!");
        HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);

        ts_type * query =(ts_type *)malloc(ts_length*sizeof(ts_type));

        FILE *dfp = fopen(queries, "rb");
        if (dfp  == NULL) {
            fprintf(stderr, "Queries file %s not found!\n",dfp);
            exit(-1);
        }
        querying_stats s;
        float* result;

        fread(query, sizeof(ts_type), ts_length, dfp);


        appr_alg.searchBaseLayerdumy(k,query, efs);
        exit(1);
cout<<endl;
    }

    //OUTDEGREE
    else if(mode ==20){
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder doesn't exist, Please make sure to give an existing index path!");

        HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);

        int minout=1000,maxout=0;
        size_t avgout = 0;
        for(int i=0;i<appr_alg.cur_element_count;i++){
            auto size = appr_alg.getListCount(appr_alg.get_linklist0(i));
            if(size > maxout)maxout = size;
            if(size < minout)minout = size;
            avgout+=size;
            if(size == 0){
                auto t = appr_alg.get_linklist0(i);
                cout << i<< " "<<t[0] <<" " << t[1]<<" "  << t[2]<<endl;
            }
            if(i == appr_alg.cur_element_count/3 +1 ){
                std::cout << "1/3 Min out "<< minout <<" ; Max out "<< maxout
                          <<" AVG "<< (double)avgout/i<<std::endl;
            }
            if(i == (appr_alg.cur_element_count/3)*2 +1 )
                std::cout << "2/3 Min out "<< minout <<" ; Max out "<< maxout
                          <<" AVG "<< (double)avgout/i<<std::endl;

        }
        std::cout << "3/3 Min out "<< minout <<" ; Max out "<< maxout
                  <<" AVG "<< (double)avgout/appr_alg.cur_element_count<<std::endl;
    }
    else if(mode ==21){
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder doesn't exist, Please make sure to give an existing index path!");

        HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);
        auto diam = findDiameter(appr_alg);
        cout << "Diameter : "<<diam<<endl;
    }
    else if(mode ==22){
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder doesn't exist, Please make sure to give an existing index path!");

        HierarchicalNSW<ts_type> appr_alg(&l2space, index_full_filename, false);
 cout << "Approx Diameter on "<< M <<" nodes : ";        
auto diam = findApproxDiameter(appr_alg, M);
       cout<<diam<<endl;
    }
    else if(mode ==23){

        dataset_size =  66666667;
        int dim = 96;


        float * distss = new float[dataset_size];

        auto s_build = new PTK::Timer();
    auto dist = l2space.get_dist_func();
    float * data =(float *) malloc(ts_length*dataset_size*sizeof(float));
        read_data(dataset, &data,ts_length,dataset_size, 0);
        cout << "data loaded"<<endl;
    for(int i =0;i<dataset_size;i++){
        distss[i] =  sqrt(dist((float *)data,(float * )( data + i*ts_length), (void * )&ts_length));
    }
        s_build->printElapsedTime(std::string("TOTAL TIME").c_str());

    }
    else
    {
        fprintf(stderr, "Please use a valid mode. run srs --help for more information. \n");
        return -1;
    }
    peak_memory_footprint();
    return 0;
}






void query_workload_kdt(size_t vecsize,
                           size_t qsize,
                           HierarchicalNSW<ts_type> &appr_alg,
                           size_t vecdim,
                           vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
                           size_t k,
                           char * queries,
                           size_t efs, uint **kdeps) {
    size_t correct = 0;
    size_t total = 0;

    ts_type * query =(ts_type *)malloc(vecdim*sizeof(ts_type));

    FILE *dfp = fopen(queries, "rb");
    if (dfp  == NULL) {
        fprintf(stderr, "Queries file %s not found!\n",dfp);
        exit(-1);
    }

    //#pragma omp parallel for
    querying_stats s;
    float* result;

    for (int i = 0; i < qsize; i++) {
        fread(query, sizeof(ts_type), vecdim, dfp);
        appr_alg.setEf(efs);
            result = appr_alg.searchGraphBsltreeps(query, k, kdeps[i],s);
        printKNN(result, k, s);
        s.time_cnmd=0;s.time_update_knn=0;s.time_leaves_search=0;s.time_routing=0;s.time_layer0=0;s.time_pq=0;
        s.num_hops_bsl=0;s.distance_computations_hrl=0;s.num_hops_hrl=0;s.distance_computations_bsl=0;
        s.saxdist_computations_bsl=0;s.saxdist_computations_hsl=0;
    }

}


void add_data(HierarchicalNSW<ts_type> &appr_alg, ts_type *data, unsigned int ts_length, unsigned int data_size,
              unsigned int label_offset, int rng, float prune,int cnt, int range)

{
    for (int i = 0; i < 1; i++) {
        appr_alg.addPoint((void *) (data + ts_length * i), (size_t) i+label_offset, rng, prune, cnt, range);
    }

#pragma omp parallel for
    for (int i = 1; i < data_size; i++) {
        appr_alg.addPoint((void *) (data + ts_length * i), (size_t) i+label_offset, rng, prune, cnt, range);
    }
}


void add_data_ksrep(HierarchicalNSW<ts_type> &appr_alg, ts_type *data, unsigned int ts_length, unsigned int data_size,
              unsigned int label_offset, int rng, float prune)

{
    for (int i = 0; i < 1; i++) {
        appr_alg.addPoint_ksrep((void *) (data + ts_length * i), (size_t) i+label_offset, rng, prune);
    }

#pragma omp parallel for
    for (int i = 1; i < data_size; i++) {
        appr_alg.addPoint_ksrep((void *) (data + ts_length * i), (size_t) i+label_offset, rng, prune);
    }
}
void read_data(char * dataset,
               ts_type ** pdata,
               unsigned int ts_length,
               unsigned int data_size,
               unsigned int offset)
{

    FILE *dfp = fopen(dataset, "rb");
    fseek(dfp, 0, SEEK_SET);
    fseek(dfp, (unsigned long) offset * ts_length * sizeof(ts_type), SEEK_SET);
    fread(*pdata, sizeof(ts_type), data_size * ts_length, dfp);
    fclose(dfp);
}


void
query_workload(//ts_type *massQ,
        size_t vecsize,
        size_t qsize,
        HierarchicalNSW<ts_type> &appr_alg,
        size_t vecdim,
        vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
        size_t k,
        char * queries,
        size_t efs,
        bool flatt,int sims)
{
    size_t correct = 0;
    size_t total = 0;

    ts_type * query =(ts_type *)malloc(vecdim*sizeof(ts_type));

    FILE *dfp = fopen(queries, "rb");
    if (dfp  == NULL) {
        fprintf(stderr, "Queries file %s not found!\n",dfp);
        exit(-1);
    }

    //#pragma omp parallel for
    querying_stats s;
    float* result;
    unsigned int * ids;

    for (int i = 0; i < qsize; i++) {


        fread(query, sizeof(ts_type), vecdim, dfp);

        appr_alg.setEf(efs);


        if(flatt) {

            auto tmp = appr_alg.searchGraphBsl(query, k, s);
            result = tmp.first;
            ids = tmp.second;
        }
        else {

            auto tmp = appr_alg.searchGraph(query, k, s);
            result = tmp.first;ids = tmp.second;
            ids = tmp.second;
        }


        printKNN(result, k, s,ids);

        s.time_cnmd=0;s.time_update_knn=0;s.time_leaves_search=0;s.time_routing=0;s.time_layer0=0;s.time_pq=0;
        s.num_hops_bsl=0;s.distance_computations_hrl=0;s.num_hops_hrl=0;s.distance_computations_bsl=0;
        s.saxdist_computations_bsl=0;s.saxdist_computations_hsl=0;
    }

}


void
query_workloadrdseed(
        size_t vecsize,
        size_t qsize,
        HierarchicalNSW<ts_type> &appr_alg,
        size_t vecdim,
        vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
        size_t k,
        char * queries,
        size_t efs)
{
    size_t correct = 0;
    size_t total = 0;

    ts_type * query =(ts_type *)malloc(vecdim*sizeof(ts_type));

    FILE *dfp = fopen(queries, "rb");
    if (dfp  == NULL) {
        fprintf(stderr, "Queries file %s not found!\n",dfp);
        exit(-1);
    }

    querying_stats s;
    float* result;
    appr_alg.setEf(efs);
    for (int i = 0; i < qsize; i++) {


        fread(query, sizeof(ts_type), vecdim, dfp);




            result = appr_alg.searchGraphBslrdseed(query, k, s);


        printKNN(result, k, s);

        s.time_cnmd=0;s.time_update_knn=0;s.time_leaves_search=0;s.time_routing=0;s.time_layer0=0;s.time_pq=0;
        s.num_hops_bsl=0;s.distance_computations_hrl=0;s.num_hops_hrl=0;s.distance_computations_bsl=0;
        s.saxdist_computations_bsl=0;s.saxdist_computations_hsl=0;
    }

}


void printKNN(float * results, int k, querying_stats stats,unsigned  int * ids){
    if(ids == nullptr){
        cout << "----------"<<k<<"-NN RESULTS----------- | visited nodes : \n";
        for(int i = 0 ; i < k ; i++){
            if(i ==0)
            printf( " K N°%i  => Distance : %f | Node ID : %lu | Time  : %f |  "
                    "Total DC : %lu | HDC : %lu | BDC : %lu | "
                    "Total LBDC : %lu | HLBDC : %lu | BLBDC : %lu | HLhops : %lu | BLhops : %lu \n",i+1,sqrt(results[i]),
                    0,stats.time_leaves_search,stats.distance_computations_bsl+stats.distance_computations_hrl
                    ,stats.distance_computations_hrl,stats.distance_computations_bsl,
                    stats.saxdist_computations_hsl+stats.saxdist_computations_bsl,stats.saxdist_computations_hsl,
                    stats.saxdist_computations_bsl, stats.num_hops_hrl, stats.num_hops_bsl);
            else
                printf( " K N°%i  => Distance : %f | Node ID : %lu | Time  : 0 |  "
                        "Total DC : 0 | HDC : 0 | BDC : 0 | "
                        "Total LBDC : 0 | HLBDC : 0 | BLBDC : 0 | HLhops : 0 | BLhops : 0 \n",i+1,sqrt(results[i]));


        }
    }else{
        cout << "----------"<<k<<"-NN RESULTS----------- | visited nodes : \n";
        for(int i = 0 ; i < k ; i++){
            if(i ==0)
                printf( " K N°%i  => Distance : %f | Node ID : %lu | Time  : %f |  "
                        "Total DC : %lu | HDC : %lu | BDC : %lu | "
                        "Total LBDC : %lu | HLBDC : %lu | BLBDC : %lu | HLhops : %lu | BLhops : %lu \n",i+1,sqrt(results[i]),
                        ids[i],stats.time_leaves_search,stats.distance_computations_bsl+stats.distance_computations_hrl
                        ,stats.distance_computations_hrl,stats.distance_computations_bsl,
                        stats.saxdist_computations_hsl+stats.saxdist_computations_bsl,stats.saxdist_computations_hsl,
                        stats.saxdist_computations_bsl, stats.num_hops_hrl, stats.num_hops_bsl);
            else
                printf( " K N°%i  => Distance : %f | Node ID : %lu | Time  : %f |  "
                        "Total DC : %lu | HDC : %lu | BDC : %lu | "
                        "Total LBDC : %lu | HLBDC : %lu | BLBDC : %lu | HLhops : %lu | BLhops : %lu  \n",i+1,sqrt(results[i]),
                        ids[i],0,0,0,0,0,0,0,0,0);
        }
    }

}
