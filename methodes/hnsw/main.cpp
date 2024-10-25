#include <iostream>
#include "include/hnswlib/hnswlib.h"
#include <getopt.h>

using namespace hnswlib;
using namespace std;
#include <H5Cpp.h>
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
#include "sax.h"
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

void printKNN(float * results, int k,  querying_stats stats);


void read_data(char * dataset,
               ts_type ** pdata,
               unsigned int ts_length,
               unsigned int data_size,
               unsigned int offset);

void add_data(HierarchicalNSW<ts_type> &appr_alg,
              ts_type * data,
              unsigned int ts_length,
              unsigned int data_size, unsigned int label_offset, float rng = 0);


void loadsims(HierarchicalNSW<ts_type> &g, unsigned int ts_length, char *filename);

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
    int sims = 0;
    bool flatt = 0;
    int paa_segments = 16;
    int sax_bit_cardinality = 8;

    float rng = 0;
    while (1) {
        static struct option long_options[] = {
                {"efconstruction",  required_argument, 0, 'b'},
                {"rng",           required_argument, 0, 'r'},
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
                {"sims",            required_argument, 0, 's'},
                {"paa-segments",    required_argument, 0, 'ps'},
                {"bit-sax",         required_argument, 0, 'bs'},
                {"help",            no_argument,       0, '?'}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        int c = getopt_long(argc, argv, "",
                            long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
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

            case 'c':
                flatt = (bool) atoi(optarg);
                break;

            case 'p':
                index_path = optarg;
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

            case 't':
                ts_length = atoi(optarg);
                break;

            case 'm':
                M = atoi(optarg);
                break;

            case 's':
                sims = atoi(optarg);
                break;

            case 'ps':
                paa_segments = atoi(optarg);
                break;
            case 'bs':
                sax_bit_cardinality = atoi(optarg);
                break;
            case 'r':
                rng = atoi(optarg);
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
/*

    H5::Exception::dontPrint();
    H5::H5File file(queries, H5F_ACC_RDONLY);
//    H5::DataSet pca96_dataset = file.openDataSet("pca96");
//    H5::DataSpace dataspace = pca96_dataset.getSpace();
//    hsize_t dims[2];
//    dataspace.getSimpleExtentDims(dims, NULL);
//    std::cout << "Number of rows: " << dims[0] << std::endl;
//    std::cout << "Number of columns: " << dims[1] << std::endl;
//    std::vector<float> data(dims[0] * dims[1]);
//    pca96_dataset.read(data.data(), H5::PredType::NATIVE_FLOAT);
//
//    for(int i =0; i < 96 ; i ++)cout << data[i] << " ";



    // Get the number of objects in the root group
    int numObjects = file.getNumObjs();
    // Iterate through the objects in the root group
    for (int i = 0; i < numObjects; ++i) {
        // Get the object type and name
        H5G_obj_t objType = file.getObjTypeByIdx(i);
        std::string objName = file.getObjnameByIdx(i);

        // Check if the object is a dataset
        if (objType == H5G_DATASET) {
            std::cout << "Dataset Name: " << objName << std::endl;

            // Open the dataset
            H5::DataSet dataset = file.openDataSet(objName);

            H5::DataSpace dataspace = dataset.getSpace();
            hsize_t dims[2];
            dataspace.getSimpleExtentDims(dims, NULL);
            std::cout << "Number of rows: " << dims[0] << std::endl;
            std::cout << "Number of columns: " << dims[1] << std::endl;
            std::vector<float> data(dims[0] * dims[1]);
            dataset.read(data.data(), H5::PredType::NATIVE_FLOAT);
            // Perform operations on the dataset...
            for (int i = 0; i < 96; i++)cout << data[i] << " ";
            // Close the dataset
            dataset.close();
        }
    }
    exit(1);
    */


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
        for (i = 0; i < chunk_count; ++i)
        {
            printf ("Loading %ld vectors of chunk %ld\n", chunk_size,i+1);
            read_data(dataset,&data,ts_length,chunk_size,i*chunk_size);
            add_data(appr_alg,data, ts_length,chunk_size,i*chunk_size, rng);
        }

        if (last_chunk_size != 0)
        {
            printf ("Loading %ld vectors of the last chunk %ld\n", last_chunk_size,i+1);
            read_data(dataset,&data,ts_length,last_chunk_size,i*chunk_size);
            add_data(appr_alg,data, ts_length,last_chunk_size,i*chunk_size, rng);
        }

        t_build->printElapsedTime(std::string ("Index Building").c_str());

        t_build->restart();
        if (sims == 1){
            auto s_build = new PTK::Timer() ;
            cout << "Summarization using SAX{ SEGMENTS : "<<paa_segments<<"| SAXBITS : "<<sax_bit_cardinality<<"}..."<<endl;
            sax_type * sax_cache =  static_cast<sax_type *>(calloc(paa_segments*dataset_size, sizeof(sax_type)));

            auto ts_values_per_paa_segment = (int) ts_length / paa_segments;
            auto sax_byte_size = (sizeof(sax_type) * paa_segments);
            auto sax_alphabet_cardinality = pow(2, sax_bit_cardinality);

            {
#pragma omp parallel for
                for (int i = 0; i < dataset_size; i++) {
                    auto vec = (float *) appr_alg.getDataByInternalId(i);
                    sax_from_ts(vec, sax_cache + paa_segments * i,
                                ts_values_per_paa_segment,
                                paa_segments, sax_alphabet_cardinality,
                                sax_bit_cardinality);
                }
            }

            char *sax_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 10));
            sax_full_filename = strcpy(sax_full_filename, index_path);
            sax_full_filename = strcat(sax_full_filename, "sax.bin");

            FILE *file = fopen(sax_full_filename, "wb");
            fwrite(&paa_segments,sizeof(int),1,file);
            fwrite(&sax_bit_cardinality,sizeof(int),1,file);
            fwrite(sax_cache, sizeof(sax_type), paa_segments*dataset_size, file);
            fclose(file);

            free(sax_full_filename);
            free(sax_cache);
            s_build->printElapsedTime(std::string ("Summarization finished!").c_str());
        }


        appr_alg.saveIndex(index_full_filename);

        t_build->printElapsedTime(std::string ("Index Saving").c_str());
    }
    else if (mode == 5)  //read an existing index and execute queries
    {
        if(chdir(index_path) != 0)
            throw std::runtime_error("The index folder is not existing, Please make sure to give an  existing path to generate index sax within!");
        HierarchicalNSW<ts_type> appr_alg (&l2space,index_full_filename, false);
        auto s_build = new PTK::Timer() ;
        cout << "Summarization using SAX{ SEGMENTS : "<<paa_segments<<"| SAXBITS : "<<sax_bit_cardinality<<"}..."<<endl;
        sax_type * sax_cache =  static_cast<sax_type *>(calloc(paa_segments*dataset_size, sizeof(sax_type)));
        auto ts_values_per_paa_segment = (int) ts_length / paa_segments;
        auto sax_byte_size = (sizeof(sax_type) * paa_segments);
        auto sax_alphabet_cardinality = pow(2, sax_bit_cardinality);

        {
#pragma omp parallel for
            for (int i = 0; i < dataset_size; i++) {
                auto vec = (float *) appr_alg.getDataByInternalId(i);
                sax_from_ts(vec, sax_cache + paa_segments * i,
                            ts_values_per_paa_segment,
                            paa_segments, sax_alphabet_cardinality,
                            sax_bit_cardinality);
            }
        }

        char *sax_full_filename = (char *) malloc(sizeof(char) * (strlen(index_path) + 10));
        sax_full_filename = strcpy(sax_full_filename, index_path);
        sax_full_filename = strcat(sax_full_filename, "sax.bin");

        FILE *file = fopen(sax_full_filename, "wb");
        fwrite(&paa_segments,sizeof(int),1,file);
        fwrite(&sax_bit_cardinality,sizeof(int),1,file);
        fwrite(sax_cache, sizeof(sax_type), paa_segments*dataset_size, file);
        fclose(file);

        free(sax_full_filename);
        free(sax_cache);
        s_build->printElapsedTime(std::string ("Summarization finished!").c_str());

    }
    else if (mode == 1)  //read an existing index and execute queries
    {
        HierarchicalNSW<ts_type> appr_alg (&l2space,index_full_filename, false);
        if(sims){
            loadsims(appr_alg,ts_length,index_path);
        }
        auto s_build = new PTK::Timer() ;
        vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
        query_workload(
                (size_t)dataset_size,
                (size_t)queries_size,
                appr_alg,
                (size_t)ts_length,
                answers,
                (size_t)k,
                queries,
                (size_t)efs, flatt,sims);

        s_build->printElapsedTime(std::string ("TOTAL TIME").c_str());
    }
    else if(mode ==2)//PARALLEL SEARCH
    {
        HierarchicalNSW<ts_type> appr_alg (&l2space,index_full_filename, false);
        if(sims)
            loadsims(appr_alg,ts_length,index_path);

        auto s_build = new PTK::Timer() ;
        vector<std::priority_queue<std::pair<ts_type, labeltype >>> answers;
        query_workload_IQP(
                (size_t)dataset_size,
                (size_t)queries_size,
                appr_alg,
                (size_t)ts_length,
                answers,
                (size_t)k,
                queries,
                (size_t)efs, flatt,sims);
        s_build->printElapsedTime(std::string ("TOTAL TIME").c_str());
    }
    else
    {
        fprintf(stderr, "Please use a valid mode. run srs --help for more information. \n");
        return -1;
    }

    return 0;
}

void loadsims(HierarchicalNSW<ts_type> &g, unsigned int ts_length, char *filename) {
    char *sax_full_filename = (char *) malloc(sizeof(char) * (strlen(filename) + 10));
    sax_full_filename = strcpy(sax_full_filename, filename);
    sax_full_filename = strcat(sax_full_filename, "sax.bin");
    FILE *file = fopen(sax_full_filename, "r");
    fread(&g.paa_segments,sizeof(int),1,file);
    fread(&g.sax_bit_cardinality,sizeof(int),1,file);


    g.sax_cache =  static_cast<sax_type *>(calloc(g.paa_segments*g.cur_element_count, sizeof(sax_type)));
    fread(g.sax_cache,sizeof(sax_type),g.paa_segments*g.cur_element_count,file);


    g.max_sax_cardinalities = static_cast<sax_type *>(malloc(sizeof(sax_type) * g.paa_segments));
    for (int i = 0; i < g.paa_segments; i++)g.max_sax_cardinalities[i] = g.sax_bit_cardinality;

    g.sax_alphabet_cardinality = pow(2, g.sax_bit_cardinality);
    g.mindist_sqrt = ((float) ts_length /
                      (float) g.paa_segments);

}

void add_data(HierarchicalNSW<ts_type> &appr_alg,
              ts_type * data,
              unsigned int ts_length,
              unsigned int data_size,unsigned int label_offset,float rng )

{
    for (int i = 0; i < 1; i++) {
        appr_alg.addPoint((void *) (data + ts_length * i), (size_t) i+label_offset,rng);
    }

#pragma omp parallel for
    for (int i = 1; i < data_size; i++) {
        appr_alg.addPoint((void *) (data + ts_length * i), (size_t) i+label_offset, rng);
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
    float * qpaa;
    if(sims)
    qpaa = new float[appr_alg.paa_segments];

    for (int i = 0; i < qsize; i++) {


        fread(query, sizeof(ts_type), vecdim, dfp);

        appr_alg.setEf(efs);
//        auto start = std::chrono::high_resolution_clock::now();
if(sims){
    // sims : 1 only bsl, 2 only hrl, 3 both bsl and hrl
    paa_from_ts(query,qpaa,appr_alg.paa_segments,(int)appr_alg.mindist_sqrt);
    result = appr_alg.searchGraphSax(query, qpaa, k, s, sims);
}
else  if(flatt) {

            result = appr_alg.searchGraphBsl(query, k, s);
        }
        else {

            result = appr_alg.searchGraph(query, k, s);
        }


        printKNN(result, k, s);

        s.time_cnmd=0;s.time_update_knn=0;s.time_leaves_search=0;s.time_routing=0;s.time_layer0=0;s.time_pq=0;
        s.num_hops_bsl=0;s.distance_computations_hrl=0;s.num_hops_hrl=0;s.distance_computations_bsl=0;
        s.saxdist_computations_bsl=0;s.saxdist_computations_hsl=0;
    }

}

void
query_workload_IQP(
        size_t vecsize,
        size_t qsize,
        HierarchicalNSW<ts_type> &appr_alg,
        size_t vecdim,
        vector<std::priority_queue<std::pair<ts_type, labeltype >>> &answers,
        size_t k,
        char * queriespath,
        size_t efs,
        bool flatt,int sims)
{

    ts_type * queries = new float[qsize*vecdim];

    FILE *dfp = fopen(queriespath, "rb");
    if (dfp  == NULL) {
        fprintf(stderr, "Queries file %s not found!\n",dfp);
        exit(-1);
    }

    fread(queries, sizeof(ts_type), vecdim*qsize, dfp);
    fclose(dfp);
    appr_alg.setEf(efs);
    float** results = new float*[qsize];
    querying_stats * stats = new querying_stats [qsize];


    {
#pragma omp parallel for
        for (int i = 0; i < qsize; i++) {
            if (sims) {
                // sims : 1 only bsl, 2 only hrl, 3 both bsl and hrl
                float *qpaa = new float[appr_alg.paa_segments];
                paa_from_ts(queries + i * vecdim, qpaa, appr_alg.paa_segments, (int) appr_alg.mindist_sqrt);
                results[i] = appr_alg.searchGraphSax(queries + i * vecdim, qpaa, k, stats[i], sims);
            } else if (flatt) {

                results[i] = appr_alg.searchGraphBsl(queries + i * vecdim, k, stats[i]);
            } else {

                results[i] = appr_alg.searchGraph(queries + i * vecdim, k, stats[i]);
            }
        }

    }
    for (int i = 0; i < qsize; i++){
        printKNN(results[i], k, stats[i]);
    }
    delete results;
    delete stats;
    delete queries;




}

void printKNN(float * results, int k, querying_stats stats){
    cout << "----------"<<k<<"-NN RESULTS----------- | visited nodes : \n";
    for(int i = 0 ; i < k ; i++){
        printf( " K N°%i  => Distance : %f | Node ID : %lu | Time  : %f |  "
                "Total DC : %lu | HDC : %lu | BDC : %lu | "
                "Total LBDC : %lu | HLBDC : %lu | BLBDC : %lu | \n",i+1,sqrt(results[i]),
                0,stats.time_leaves_search,stats.distance_computations_bsl+stats.distance_computations_hrl
                ,stats.distance_computations_hrl,stats.distance_computations_bsl,
                stats.saxdist_computations_hsl+stats.saxdist_computations_bsl,stats.saxdist_computations_hsl,stats.saxdist_computations_bsl);

    }
}
