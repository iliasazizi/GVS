//
// Created by iazizi on 19‏/12‏/2021.
//

#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include "sys/stat.h"
#include <fstream>
#include <ctime>
#include <malloc.h>
#include <efanna2e/index_nsg.h>
#include <efanna2e/util.h>



using namespace std;
namespace po = boost::program_options;
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

int main(int argc, char** argv){
    string data_path ;
    string index_path ;
    string init_graph ;
    string query_path;

    unsigned int num_points;
    unsigned int num_query;
    unsigned int ts_len ;
    unsigned short mode;

    unsigned L,R,C,knn ;
    unsigned short num_threads;


    po::options_description desc_visible("General options");
    desc_visible.add_options()
            ("help,h", "produce help message.")
            ("version,v", "print version information.")
            ("dataset", po::value(&data_path), "input path")
            ("dataset-size", po::value(&num_points), "number of data-points")
            ("queries", po::value(&query_path), "query path")
            ("queries-size", po::value(&num_query), "number of query")
            ("timeseries-size", po::value(&ts_len), "dimension")
            ("index-path", po::value(&index_path), "index path")
            ("init-graph", po::value(&init_graph), "init graph path")
            ("mode", po::value(&mode), "0 : build | 1 : search")
            ("K", po::value(&knn)->default_value(10), "number of nearest neighbor(outdegree for building)")
            ("L", po::value(&L)->default_value(30), "Size of the candidate set, larger  is more accurate, but slower, L>=Knn ")
            ("R", po::value(&R)->default_value(25), "Check (larger is more accurate but slower, R >= K)   ")
            ("C", po::value(&C)->default_value(10), "S (larger is more accurate but slower)  ")
            ;

    po::options_description desc("Allowed options");
    desc.add(desc_visible);

    po::positional_options_description p;
    p.add("data", 1);
    p.add("output", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);



    auto graph_path = index_path + "index.graph";
    if(mode ==0){
        float* data = NULL;
        load_data(data_path,data,num_points,ts_len);
//        data = efanna2e::data_align(data, num_points, ts_len);//one must align the data before build

        if(chdir(index_path.c_str()) == 0)
            throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");
        mkdir(index_path.c_str() , 07777);

        efanna2e::Parameters paras;
        paras.Set<unsigned>("L", L);
        paras.Set<unsigned>("R", R);
        paras.Set<unsigned>("C", C);
        paras.Set<std::string>("nn_graph_path", init_graph);


        // data_load = efanna2e::data_align(data_load, points_num, dim);//one must
        // align the data before build

        cout << "[BUILDING NSG] : {L:"<<L<<", R:"<<R<<", C:"<<C<<"} Init GRAPH : "<<init_graph<<endl;

        auto s = std::chrono::high_resolution_clock::now();

        efanna2e::IndexNSG index(ts_len,num_points, efanna2e::L2, nullptr);
        index.Build(num_points, data, paras);

        auto e = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = e - s;


        index.Save(graph_path.c_str());

        cout << "NSG building finished!\n Graph Path => "<<graph_path<<"\n Total time => "<<diff.count()<<"(sec)"<<endl;
        delete data;
    }else if(mode==1){
        float* data = nullptr;float * query = nullptr;
        load_data(data_path,data,num_points,ts_len);
        load_data(query_path,query,num_query,ts_len);
//        data = efanna2e::data_align(data, num_points, ts_len);//one must align the data before build
//        data = efanna2e::data_align(query, num_query, ts_len);//one must align the data before build

        efanna2e::Parameters paras;
        paras.Set<unsigned>("L_search", L);
        paras.Set<unsigned>("P_search", L);

        cout << "Load index...";
        auto start = std::chrono::high_resolution_clock::now();
        efanna2e::IndexNSG index(ts_len, num_points, efanna2e::L2, nullptr);
        index.Load(graph_path.c_str());
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end-start;
        cout<< "Done! time : "<<diff.count()<<endl;
        for (unsigned i = 0; i < num_query; i++) {
            index.Search(query + i * ts_len, data, knn, paras, nullptr);
        }
        delete data;
        delete query;}
    else if(mode ==2) {
        float* data = nullptr;float * query = nullptr;
        load_data(data_path,data,num_points,ts_len);
        load_data(query_path,query,num_query,ts_len);
//        data = efanna2e::data_align(data, num_points, ts_len);//one must align the data before build
//        data = efanna2e::data_align(query, num_query, ts_len);//one must align the data before build

        efanna2e::Parameters paras;
        paras.Set<unsigned>("L_search", L);
        paras.Set<unsigned>("P_search", L);

        cout << "Load index...";
        auto start = std::chrono::high_resolution_clock::now();
        efanna2e::IndexNSG index(ts_len, num_points, efanna2e::FAST_L2, nullptr);
        index.Load(graph_path.c_str());
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end-start;
        cout<< "Done! time : "<<diff.count()<<endl;
        cout << "Optimize index...";
        start = std::chrono::high_resolution_clock::now();
        index.OptimizeGraph(data);
        end = std::chrono::high_resolution_clock::now();
        diff = end - start;
        cout << "Done! time : " << diff.count() << endl;


        for (unsigned i = 0; i < num_query; i++) {
            index.SearchWithOptGraph(query + i * ts_len, knn, paras, nullptr);
        }

    }



    return 0;
}