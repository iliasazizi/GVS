#include <index.h>
#include <omp.h>
#include <string.h>
#include "utils.h"

#ifndef _WINDOWS
#include <sys/mman.h>
#include <unistd.h>
#else
#include <Windows.h>
#endif
#include "memory_mapper.h"
#include <random>
#include <iomanip>
#include <type_traits>
#include <iostream>
//#include <boost/program_options.hpp>

#include "sys/stat.h"

using namespace std;

int build_in_memory_index(const std::string&     data_path, const unsigned num_data, const unsigned dim,
                          const diskann::Metric& metric, const unsigned R,                           const unsigned L, const unsigned C, const float alpha,
                          const std::string& save_path,
                          const unsigned     num_threads) {
  diskann::Parameters paras;
  paras.Set<unsigned>("R", R);
  paras.Set<unsigned>("L", L);
  paras.Set<unsigned>(
      "C", C);  // maximum candidate set size during pruning procedure
  paras.Set<float>("alpha", alpha);
  paras.Set<bool>("saturate_graph", 0);
  paras.Set<unsigned>("num_threads", num_threads);
    auto              s = std::chrono::high_resolution_clock::now();
  diskann::Index<float> index(metric, data_path.c_str(),num_data,dim);

  index.build(paras);
  std::chrono::duration<double> diff =
      std::chrono::high_resolution_clock::now() - s;

  std::cout << "Indexing time: " << diff.count() << " \npath index: "<< save_path<< "\n";
  s = std::chrono::high_resolution_clock::now();
  index.save((save_path+"index.bin").c_str());
diff =
      std::chrono::high_resolution_clock::now() - s;

  std::cout << "Saving time: " << diff.count()<<" \n";
  return 0;
}

int search_memory_index(string data_file, unsigned num_data,string memory_index_file, unsigned dim,
                        string query_bin, unsigned num_query, _u64 L, _u64 K) {
  float*                query = nullptr;

  size_t            query_num, query_dim, query_aligned_dim;
  std::vector<_u64> Lvec;

  diskann::Metric metric;

  metric = diskann::Metric::L2;

  _u64        recall_at = K;

  Lvec.push_back(L);


  diskann::load_data<float>(query_bin,query,num_query,dim,query_aligned_dim);
//  diskann::load_aligned_bin<float>(query_bin, query, query_num, query_dim,
//                               query_aligned_dim);

    query_num=num_query;query_dim=dim;
  std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
  std::cout.precision(2);

  diskann::Index<float> index(metric, data_file.c_str(),num_data,dim);
  index.load(memory_index_file.c_str());  // to load NSG
  std::cout << "Index loaded" << std::endl;



  if (metric == diskann::FAST_L2)
    index.optimize_graph();

  diskann::Parameters paras;


  std::vector<std::vector<uint32_t>> query_result_ids(1);
  std::vector<std::vector<float>>    query_result_dists(1);

    query_result_ids[0].resize(recall_at * query_num);
    //    omp_set_num_threads(num_threads);
    //#pragma omp parallel for schedule(dynamic, 1)
    for (int64_t i = 0; i < (int64_t) query_num; i++) {
        index.search(query + i * query_aligned_dim, recall_at, L,
                     query_result_ids[0].data() + i * recall_at);
  }

  diskann::aligned_free(query);
  return 0;
}


#include <getopt.h>



int findDiameter(const vector<vector<unsigned>>& graph) {
    int diameter = 0;
    int n = graph.size();

    // Function to perform BFS and find the maximum distance from a starting node
    auto bfs = [&](int start) {
        vector<bool> visited(n, false);
        queue<pair<int, int>> q; // Queue stores node and its distance
        q.push({start, 0}); // Push the starting node with distance 0
        visited[start] = true;
        int maxDistance = 0;

        while (!q.empty()) {
            auto [node, distance] = q.front();
            q.pop();
            if (distance > maxDistance) {
                maxDistance = distance;
            }

            for (unsigned neighbor : graph[node]) {
                if (!visited[neighbor]) {
                    q.push({neighbor, distance + 1});
                    visited[neighbor] = true;
                }
            }
        }

        return maxDistance;
    };

    // For each node, perform BFS and find the maximum distance
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
#include "unordered_set"
int findApproxDiameter(const vector<vector<unsigned>>& graph, int num_samples = 1000000) {
    int diameter = 0;
    int n = graph.size();

    // Random number generator
    random_device rd;
    mt19937 gen(1996); // Setting the seed to 1996
    uniform_int_distribution<unsigned> dist(0, n - 1);

    // Function to perform BFS and find the maximum distance from a starting node
    auto bfs = [&](int start) {
        vector<bool> visited(n, false);
        queue<pair<int, int>> q; // Queue stores node and its distance
        q.push({start, 0}); // Push the starting node with distance 0
        visited[start] = true;
        int max_distance = 0;

        while (!q.empty()) {
            auto [node, distance] = q.front();
            q.pop();
            if (distance > max_distance) {
                max_distance = distance;
            }

            for (unsigned neighbor : graph[node]) {
                if (!visited[neighbor]) {
                    q.push({neighbor, distance + 1});
                    visited[neighbor] = true;
                }
            }
        }

        return max_distance;
    };

    // Sample nodes for approximation
    unordered_set<unsigned> sampled_nodes;
    while (sampled_nodes.size() < num_samples) {
        sampled_nodes.insert(dist(gen));
    }

    // Copy the sampled nodes from unordered_set to a vector
    vector<unsigned> sampled_nodes_vec(sampled_nodes.begin(), sampled_nodes.end());

    // For each sampled node, perform BFS and find the maximum distance
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

int main(int argc, char *argv[]){
    string                  data_file;
    string                  index_file;
    string                  query_file;
    int                     L=200;
    int                     K=100;
    float                   alpha=1.5;
    unsigned                num_threads;
    bool                    mode;
    unsigned C = 500;
    unsigned num_data,num_query,dim;

    while (1) {
        static struct option long_options[] = {
                {"dataset",          required_argument, 0, 'd'},
                {"dataset_size",     required_argument, 0, 'z'},
                {"timeseries_size",  required_argument, 0, 't'},
                {"query",          required_argument, 0, 'q'},
                {"query_size",     required_argument, 0, 'f'},
                {"index",       required_argument, 0, 'p'},
                {"K",          required_argument, 0, 'k'},
                {"L",          required_argument, 0, 'l'},
                {"C",          required_argument, 0, 'c'},
                {"alpha",                required_argument, 0, 'a'},
                {"nthrds",                required_argument, 0, 'th'},
                {"mode",             required_argument, 0, 'x'},
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "",
                            long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
            case 'a':
                alpha = atof(optarg);
                break;
            case 'th':
                num_threads = atoi(optarg);
                break;
            case 'c':
                C = atoi(optarg);
                break;
            case 'k':
                K = atoi(optarg);
                break;
            case 'l':
                L = atoi(optarg);
                break;
            case 'q':
                query_file = optarg;
                break;
            case 'f':
                num_query = atoi(optarg);
                break;
            case 'd':
                data_file = optarg;
                break;
            case 'p':
                index_file = optarg;
                break;
            case 'x':
                mode = atoi(optarg);
                break;
            case 'z':
                num_data = atoi(optarg);
                break;
            case 't':
                dim = atoi(optarg);
                break;
            default:
                exit(-1);
                break;
        }
    }

    if(mode ==0){
        auto metric = diskann::Metric::L2;
        if(chdir(index_file.c_str()) == 0)
            throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");
        mkdir(index_file.c_str() , 07777);
        build_in_memory_index(data_file,num_data,dim, metric, K, L, C, alpha, index_file,
                              num_threads);
    }else if(mode==1){
        search_memory_index(data_file,num_data,index_file+"index.bin",dim, query_file,num_query,L,K);
    } else if(mode ==20){
      diskann::Metric metric;
      metric = diskann::Metric::L2;

      diskann::Index<float> index(metric, data_file.c_str(),num_data,dim);
      auto indexp = index_file+"index.bin";
      index.load(indexp.c_str());

      auto n = index._final_graph.size();

      int minout=1000,maxout=0;
      size_t avgout = 0;
      for(int i=0;i<n;i++){
          auto size = index._final_graph[i].size();
          if(size > maxout)maxout = size;
          if(size < minout)minout = size;
          avgout+=size;
          if(size == 0){
              auto t =  index._final_graph[i];
              cout << i<< " "<<t[0] <<" " << t[1]<<" "  << t[2]<<endl;
          }
          if(i == n/3 +1 ){
              std::cout << "1/3 Min out "<< minout <<" ; Max out "<< maxout
                        <<" AVG "<< avgout/i<<std::endl;
          }
          if(i == (n/3)*2 +1 )
              std::cout << "2/3 Min out "<< minout <<" ; Max out "<< maxout
                        <<" AVG "<< avgout/i<<std::endl;

      }
      std::cout << "3/3 Min out "<< minout <<" ; Max out "<< maxout
                <<" AVG "<< avgout/n<<std::endl;
  }
  else if(mode ==21){

      diskann::Metric metric;
      metric = diskann::Metric::L2;

      diskann::Index<float> index(metric, data_file.c_str(),num_data,dim);
      auto indexp = index_file+"index.bin";
      index.load(indexp.c_str());

      auto diam = findDiameter(index._final_graph);
      cout << "Diameter :" << diam << endl;


  }
  else if(mode ==22){

      diskann::Metric metric;
      metric = diskann::Metric::L2;

      diskann::Index<float> index(metric, data_file.c_str(),num_data,dim);
      auto indexp = index_file+"index.bin";
      index.load(indexp.c_str());

      auto diam = findApproxDiameter(index._final_graph, K);
      cout << "Approx Diameter on "<<K<<" nodes :" << diam << endl;


  }

}
/*
#include "boost/program_options.hpp"
using namespace boost;
namespace po = boost::program_options;
int main (int argc, char *argv[]) {
  string                  data_file;
  string                  index_file;
  string                  query_file;
  int                     L=200;
  int                     K=100;
  float                   alpha=1.5;
  unsigned                num_threads;
  bool                    mode;
  unsigned C = 750;
  unsigned num_data,num_query,dim;

  po::options_description desc_visible("General options");
  desc_visible.add_options()("help,h", "produce help message.")(
      "version,v", "print version information.")(
      "dataset", po::value(&data_file), "data path")(
      "query", po::value(&query_file), "query path")(
      "index", po::value(&index_file), "index path")(
      "K", po::value(&K)->default_value(100), "number of nearest neighbor")(
      "L", po::value(&L)->default_value(200), "number of nearest neighbor")(
      "C", po::value(&C)->default_value(750), "number of nearest neighbor")(
      "alpha", po::value(&alpha)->default_value(1.5), "alpha RNG")(
      "dataset_size", po::value(&num_data), "datasetsize")(
      "query_size", po::value(&num_query), "queyrsize")(
      "timeseries_size", po::value(&dim), "dim")(
      "mode", po::value(&mode), "mode : 0 build index, 1 search");

  po::options_description desc_hidden("Expert options");
  desc_hidden.add_options()("num_threads",
                            po::value(&num_threads)->default_value(1),
                            "Nomber of threads for fast building");
  po::options_description desc("Allowed options");
  desc.add(desc_visible).add(desc_hidden);
  po::positional_options_description p;
  p.add("dataset", 1);
  p.add("index", 1);
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  po::notify(vm);


    if(mode ==0){
        auto metric = diskann::Metric::L2;
        if(chdir(index_file.c_str()) == 0)
            throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");
        mkdir(index_file.c_str() , 07777);
        build_in_memory_index(data_file,num_data,dim, metric, K, L, C, alpha, index_file,
                              num_threads);
    }else if(mode==1){
        search_memory_index(data_file,num_data,index_file+"index.bin",dim, query_file,num_query,L,K);
    }

}*/
