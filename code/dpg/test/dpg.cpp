#include <weavess/builder.h>
#include <iostream>
#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include "sys/stat.h"
#include <fstream>
#include <ctime>
#include <malloc.h>
namespace po = boost::program_options;
using namespace std;
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

void load_data(const std::string &path, float*& data, size_t num,int dim){// load data with sift10K pattern
    ifstream in(path.c_str(), ios::binary);
    if(!in.is_open()){cout<<"open file error"<<endl;exit(-1);}

    data = new float[(size_t)num * (size_t)dim];
    in.seekg(0, std::ios::beg);
    for(size_t i = 0; i < num; i++){
        in.read((char*)(data+i*dim),dim*4);
    }
    in.close();
}

int main(int argc, char** argv) {
    string data_path;
    string index_path;
    string query_path;

    unsigned int num_points;
    unsigned int num_query;
    unsigned int ts_len;
    unsigned int K;
    unsigned int L;
    unsigned short mode;

    unsigned int R;
    unsigned int S;
    unsigned int iters;
    unsigned int numthreads;
    auto osthreads = (std::thread::hardware_concurrency()==0)? sysconf(_SC_NPROCESSORS_ONLN) : std::thread::hardware_concurrency() -1;
    po::options_description desc_visible("General options");
    desc_visible.add_options()
            ("help,h", "produce help message.")
            ("version,v", "print version information.")
            ("dataset", po::value(&data_path), "input path")
            ("dataset-size", po::value(&num_points), "number of data-points")
            ("queries", po::value(&query_path), "query path")
            ("queries-size", po::value(&num_query), "number of query")
            ("timeseries-size", po::value(&ts_len), "dimension")
            ("index-path", po::value(&index_path), "output path")
            ("mode", po::value(&mode), "0 : build | 1 : search")
            ("K", po::value(&K)->default_value(100), "number of nearest neighbor(outdegree for building)")
            ("iters", po::value(&iters)->default_value(12), " iters")
            ("R", po::value(&R)->default_value(100), "R")
            ("S", po::value(&S)->default_value(25), "S")
            ("L", po::value(&L)->default_value(140), "Size of the candidate set, larger  is more accurate, but slower, L>=Knn ")
            ("numthreads", po::value(&numthreads)->default_value(osthreads), "num threads for parallel indexing ")
            ;

    po::options_description desc("Allowed options");
    desc.add(desc_visible);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);


    auto graph_path = index_path + "index.kgraph";
    float* data = NULL;
    load_data(data_path,data,num_points,ts_len);

    if(mode == 0 ){

        auto start = std::chrono::high_resolution_clock::now();

        cerr << ">>>>>>>>>>>>>Index Building \n";
        if(chdir(index_path.c_str()) == 0)
            throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");
        mkdir(index_path.c_str() , 07777);

        weavess::Parameters parameters;
        cerr << "\t\tParameters}>>> R:"<<R
        <<" | K:"<<K<<" | iters:"<<iters
        <<" | L:"<<L<<" | S:"<<S<<endl;

        parameters.set<unsigned>("S", S);
        parameters.set<unsigned>("R_refine", R);
        parameters.set<unsigned>("K", K);
        parameters.set<unsigned>("L", L);
        parameters.set<unsigned>("ITER", iters);
        parameters.set<unsigned>("S", S);
        parameters.set<unsigned>("R", R);

        auto *builder = new weavess::IndexBuilder(numthreads);
        auto index = builder->final_index_;
        index->setParam(parameters);
        index->setBaseData(data);
        index->setBaseLen(num_points);
        index->setBaseDim(ts_len);

        builder -> init(weavess::TYPE::INIT_RANDOM, false)
                -> refine(weavess::TYPE::REFINE_NN_DESCENT, false);
        builder -> refine(weavess::REFINE_DPG, false)
                -> save_graph(weavess::TYPE::INDEX_DPG, (graph_path.c_str()));
	peak_memory_footprint();
        auto end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        auto time = diff.count();
        cerr << ">>>>>>>>>>>>Indexing Time "<<time << endl;
    }if(mode ==1){

        weavess::Parameters parameters;
        parameters.set<unsigned>("K_search", K);
        parameters.set<unsigned>("L_search", L);

        auto *builder = new weavess::IndexBuilder(1);
        auto index = builder->final_index_;
        index->setParam(parameters);

        index->setBaseData(data);
        index->setBaseLen(num_points);
        index->setBaseDim(ts_len);
        float * query = nullptr;
        load_data(query_path,query,num_query,ts_len);
        index->setQueryData(query);
        index->setQueryLen(num_query);
        index->setQueryDim(ts_len);

        char idx[graph_path.size() + 30];
        memcpy(idx,graph_path.c_str(),graph_path.size()+4);
        builder -> load_graph(weavess::TYPE::INDEX_DPG, idx)
                -> search(weavess::TYPE::SEARCH_ENTRY_RAND, weavess::TYPE::ROUTER_GREEDY, weavess::TYPE::L_SEARCH_ASSIGN);
	peak_memory_footprint();
    }

}

