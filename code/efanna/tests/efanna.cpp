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
#include <efanna2e/index_kdtree.h>
#include <efanna2e/index_random.h>
#include <efanna2e/index_graph.h>
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
    string query_path;

    unsigned int num_points;
    unsigned int num_query;
    unsigned int ts_len ;
    unsigned short mode;

    unsigned int trees;
    int mlevel ;
    unsigned int epochs ;
    int L ;
    int R ;
    int knn ;
    int S ;



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
            ("K", po::value(&knn)->default_value(10), "number of nearest neighbor(outdegree for building)")
            ("mlevel", po::value(&mlevel)->default_value(8), " conquer-to-depeth(smaller is more accurate but slower) ")
            ("trees", po::value(&trees)->default_value(8), "number of truncated KDtrees to build")
            ("iterations", po::value(&epochs)->default_value(8), "number of iterations to build the graph ")
            ("L", po::value(&L)->default_value(30), "Size of the candidate set, larger  is more accurate, but slower, L>=Knn ")
            ("R", po::value(&R)->default_value(25), "Check (larger is more accurate but slower, R >= K)   ")
            ("S", po::value(&S)->default_value(10), "S (larger is more accurate but slower)  ")

            ;
    int search_method;
    po::options_description desc_hidden("Expert options");
    desc_hidden.add_options()
            ("search_method", po::value(&search_method)->default_value(0), "S (larger is more accurate but slower)  ")
            ;

    po::options_description desc("Allowed options");
    desc.add(desc_visible).add(desc_hidden);

    po::positional_options_description p;
    p.add("data", 1);
    p.add("output", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);



    auto tree_path  = index_path + "index.tree";
    auto graph_path = index_path + "index.graph";
    if(mode ==0){
        float* data = NULL;
        load_data(data_path,data,num_points,ts_len);
        //data = efanna2e::data_align(data, num_points, ts_len);//one must align the data before build

        if(chdir(index_path.c_str()) == 0)
            throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");
        mkdir(index_path.c_str() , 07777);

        efanna2e::Parameters paras;
        paras.Set<unsigned>("K", knn);
        paras.Set<unsigned>("nTrees", trees);
        paras.Set<unsigned>("mLevel", mlevel);
        auto indexkdtree  = new efanna2e::IndexKDtree(ts_len, num_points, efanna2e::L2, nullptr);

        ///BUILDING TREE
        cout << "[BUILDING INIT GRAPH(KDTREE)] : {K:"<<knn<<", NTrees:"<<trees<<", MLevel:"<<mlevel<<"}"<<endl;
        auto s = std::chrono::high_resolution_clock::now();
        indexkdtree->Build(num_points, data, paras);
        auto e = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = e-s;
        std::cout <<"[BUILDING INIT GRAPH(KDTREE)] : Time cost: "<< diff.count() << "\n";
        indexkdtree->Save(tree_path.c_str());
        delete indexkdtree;
        ///NNDESCENT REFINNEMENT

        paras.Set<unsigned>("L", L);
        paras.Set<unsigned>("iter", epochs);
        paras.Set<unsigned>("S", S);
        paras.Set<unsigned>("R", R);
        efanna2e::IndexRandom init_index(ts_len,num_points);
        efanna2e::IndexGraph index(ts_len, num_points, efanna2e::L2, (efanna2e::Index*)(&init_index));

        index.Load(tree_path.c_str());
        cout << "[NNDESCENT REFINEMENT] : {K:"<<knn<<", L:"<<L<<", #iterations:"<<epochs
        <<", R:"<<R<<", S:"<<S<<"}"<<endl;
        auto s2 = std::chrono::high_resolution_clock::now();

        index.RefineGraph(data, paras);

        auto e2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff2 = e2-s2;
        std::cout <<"[NNDESCENT REFINEMENT] Time cost: "<< diff2.count() << "\n";

        index.Save(graph_path.c_str());
//        index.Search()
        cout << "Efanna building finished!\n Graph Path => "<<graph_path<<"\n KDTree Path => "<<tree_path<<"\n\nTotal time => "<<diff.count()+diff2.count()<<endl;
        delete data;
    }else if(mode==1){
        efanna2e::Parameters paras;
        paras.Set<unsigned>("L", L);
        auto index = new efanna2e::IndexGraph(ts_len, num_points, efanna2e::L2, nullptr);
        index->Load((index_path+"index.graph").c_str());

        float* data = nullptr;float * query = nullptr;
        load_data(data_path,data,num_points,ts_len);
        load_data(query_path,query,num_query,ts_len);
//        data = efanna2e::data_align(data, num_points, ts_len);//one must align the data before build

        for(int i=0;i<num_query;i++)
            index->Search(query + i*ts_len, data, knn, paras,nullptr);

        delete data;
        delete query;
        delete index;
    }


    return 0;
}