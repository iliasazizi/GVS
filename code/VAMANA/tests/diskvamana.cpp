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
#include <boost/program_options.hpp>

#include "sys/stat.h"
#include "omp.h"

#include "aux_utils.h"
#include "index.h"
#include "math_utils.h"
#include "partition_and_pq.h"
#include "utils.h"
#include <atomic>
#include <cstring>
#include <iomanip>
#include <omp.h>
#include <pq_flash_index.h>
#include <set>
#include <string.h>
#include <time.h>

#include "aux_utils.h"
#include "index.h"
#include "math_utils.h"
#include "memory_mapper.h"
#include "partition_and_pq.h"
#include "timer.h"
#include "utils.h"

#ifndef _WINDOWS
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "linux_aligned_file_reader.h"
#else
#ifdef USE_BING_INFRA
#include "bing_aligned_file_reader.h"
#else
#include "windows_aligned_file_reader.h"
#endif
#endif

#define WARMUP false
template<typename T>
bool build_index(const char* dataFilePath, const char* indexFilePath,
                 const char* indexBuildParameters, diskann::Metric metric) {
  return diskann::build_disk_index<T>(dataFilePath, indexFilePath,
                                      indexBuildParameters, metric);
}
template<typename T>
int search_disk_index(  std::string index_prefix_path,
                      _u64        num_nodes_to_cache,
                      _u32        num_threads ,
                      _u32        beamwidth,
                      std::string query_bin,
                      _u64        recall_at ,
                      _u64        L ) {
  // load query bin
  T*                query = nullptr;
  unsigned*         gt_ids = nullptr;
  float*            gt_dists = nullptr;
  size_t            query_num, query_dim, query_aligned_dim, gt_num, gt_dim;
  std::vector<_u64> Lvec;

  _u32            ctr = 2;
  diskann::Metric metric;

  /*
  if (std::string(argv[ctr]) == std::string("mips"))
    metric = diskann::Metric::INNER_PRODUCT;
  else if (std::string(argv[ctr]) == std::string("l2"))
    metric = diskann::Metric::L2;
  else {
    std::cout << "Unsupported distance function. Currently only L2/ Inner "
                 "Product support."
              << std::endl;
    return -1;
  }

  if ((std::string(argv[1]) != std::string("float")) &&
      (metric == diskann::Metric::INNER_PRODUCT)) {
    std::cout << "Currently support only floating point data for Inner Product."
              << std::endl;
    return -1;
  }
*/
  metric = diskann::Metric::L2;
  ctr++;

  std::string pq_prefix = index_prefix_path + "_pq";
  std::string disk_index_file = index_prefix_path + "_disk.index";
  std::string warmup_query_file = index_prefix_path + "_sample_data.bin";

  //  std::string truthset_bin(argv[ctr++]);

  //  std::string result_output_prefix(argv[ctr++]);

  bool calc_recall_flag = false;


  /*
    if (Lvec.size() == 0) {
      diskann::cout
          << "No valid Lsearch found. Lsearch must be at least recall_at"
          << std::endl;
      return -1;
    }
  */
  diskann::cout << "Search parameters: #threads: " << num_threads << ", ";
  if (beamwidth <= 0)
    diskann::cout << "beamwidth to be optimized for each L value" << std::endl;
  else
    diskann::cout << " beamwidth: " << beamwidth << std::endl;

  diskann::load_aligned_bin<T>(query_bin, query, query_num, query_dim,
                               query_aligned_dim);
  /*
    if (file_exists(truthset_bin)) {
      diskann::load_truthset(truthset_bin, gt_ids, gt_dists, gt_num, gt_dim);
      if (gt_num != query_num) {
        diskann::cout
            << "Error. Mismatch in number of queries and ground truth data"
            << std::endl;
      }
      calc_recall_flag = true;
    }
  */
  std::shared_ptr<AlignedFileReader> reader = nullptr;
#ifdef _WINDOWS
#ifndef USE_BING_INFRA
  reader.reset(new WindowsAlignedFileReader());
#else
  reader.reset(new diskann::BingAlignedFileReader());
#endif
#else
  reader.reset(new LinuxAlignedFileReader());
#endif

  std::unique_ptr<diskann::PQFlashIndex<T>> _pFlashIndex(
      new diskann::PQFlashIndex<T>(reader, metric));

  int res = _pFlashIndex->load(num_threads, pq_prefix.c_str(),
                               disk_index_file.c_str());

  if (res != 0) {
    return res;
  }
  // cache bfs levels
  std::vector<uint32_t> node_list;
  diskann::cout << "Caching " << num_nodes_to_cache
                << " BFS nodes around medoid(s)" << std::endl;
  //_pFlashIndex->cache_bfs_levels(num_nodes_to_cache, node_list);
  _pFlashIndex->generate_cache_list_from_sample_queries(
      warmup_query_file, 15, 6, num_nodes_to_cache, num_threads, node_list);
  _pFlashIndex->load_cache_list(node_list);
  node_list.clear();
  node_list.shrink_to_fit();

  omp_set_num_threads(num_threads);

  uint64_t warmup_L = 20;
  uint64_t warmup_num = 0, warmup_dim = 0, warmup_aligned_dim = 0;
  T*       warmup = nullptr;

  if (WARMUP) {
    if (file_exists(warmup_query_file)) {
      diskann::load_aligned_bin<T>(warmup_query_file, warmup, warmup_num,
                                   warmup_dim, warmup_aligned_dim);
    } else {
      warmup_num = (std::min)((_u32) 150000, (_u32) 15000 * num_threads);
      warmup_dim = query_dim;
      warmup_aligned_dim = query_aligned_dim;
      diskann::alloc_aligned(((void**) &warmup),
                             warmup_num * warmup_aligned_dim * sizeof(T),
                             8 * sizeof(T));
      std::memset(warmup, 0, warmup_num * warmup_aligned_dim * sizeof(T));
      std::random_device              rd;
      std::mt19937                    gen(rd());
      std::uniform_int_distribution<> dis(-128, 127);
      for (uint32_t i = 0; i < warmup_num; i++) {
        for (uint32_t d = 0; d < warmup_dim; d++) {
          warmup[i * warmup_aligned_dim + d] = (T) dis(gen);
        }
      }
    }
    diskann::cout << "Warming up index... " << std::flush;
    std::vector<uint64_t> warmup_result_ids_64(warmup_num, 0);
    std::vector<float>    warmup_result_dists(warmup_num, 0);

#pragma omp parallel for schedule(dynamic, 1)
    for (_s64 i = 0; i < (int64_t) warmup_num; i++) {
      _pFlashIndex->cached_beam_search(warmup + (i * warmup_aligned_dim), 1,
                                       warmup_L,
                                       warmup_result_ids_64.data() + (i * 1),
                                       warmup_result_dists.data() + (i * 1), 4);
    }
    diskann::cout << "..done" << std::endl;
  }

  std::vector<std::vector<uint32_t>> query_result_ids(1);
  std::vector<std::vector<float>>    query_result_dists(1);

  uint32_t optimized_beamwidth = 2;



  if (beamwidth <= 0) {
    //    diskann::cout<<"Tuning beamwidth.." << std::endl;
    optimized_beamwidth =
        optimize_beamwidth(_pFlashIndex, warmup, warmup_num,
                           warmup_aligned_dim, L, optimized_beamwidth);
  } else
    optimized_beamwidth = beamwidth;

  query_result_ids[0].resize(recall_at * query_num);
  query_result_dists[0].resize(recall_at * query_num);

  diskann::QueryStats* stats = new diskann::QueryStats[query_num];

  std::vector<uint64_t> query_result_ids_64(recall_at * query_num);
  auto                  s = std::chrono::high_resolution_clock::now();

  Time start;
  float time =0;
  std::vector<float> knnresults(recall_at);
  diskann::QueryStats* stat;

  //#pragma omp parallel for schedule(dynamic, 1)
  for (_s64 i = 0; i < (int64_t) query_num; i++) {
    start = now();
    _pFlashIndex->cached_beam_search(
        query + (i * query_aligned_dim), recall_at, L,
        query_result_ids_64.data() + (i * recall_at),
        knnresults.data(),
        optimized_beamwidth, stats + i);

    time = getElapsedTime(start);
    stat = stats +i;
    std::cout << "----------"<<recall_at<<"-NN RESULTS----------- "<<std::endl;
    for (int pos = 0 ;pos<recall_at;pos++) {
      printf(
          " K N°%d  => Distance : %f | Node ID : %lu | Time  : %f | TOTAL DC : %.0f | Total hops : %.0f  \n",
          pos + 1, std::sqrt(knnresults[pos]), query_result_ids_64[pos], time,
          stat->n_cmps, stat->n_hops);
    }
  }


  diskann::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
  diskann::cout.precision(2);

  std::string recall_string = "Recall@" + std::to_string(recall_at);
  diskann::cout << "\n"
                << std::setw(6) << "L" << std::setw(12) << "Beamwidth"
                << std::setw(16) << "QPS" << std::setw(16) << "Mean Latency"
                << std::setw(16) << "99.9 Latency" << std::setw(16)
                << "Mean IOs" << std::setw(16) << "CPU (s)";
  if (calc_recall_flag) {
    diskann::cout << std::setw(16) << recall_string << std::endl;
  } else
    diskann::cout << std::endl;
  diskann::cout
      << "==============================================================="
         "==========================================="
      << std::endl;

  auto                          e = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = e - s;
  float qps = (1.0 * query_num) / (1.0 * diff.count());

  diskann::convert_types<uint64_t, uint32_t>(query_result_ids_64.data(),
                                             query_result_ids[0].data(),
                                             query_num, recall_at);

  float mean_latency = diskann::get_mean_stats(
      stats, query_num,
      [](const diskann::QueryStats& stats) { return stats.total_us; });

  float latency_999 = diskann::get_percentile_stats(
      stats, query_num, 0.999,
      [](const diskann::QueryStats& stats) { return stats.total_us; });

  float mean_ios = diskann::get_mean_stats(
      stats, query_num,
      [](const diskann::QueryStats& stats) { return stats.n_ios; });

  float mean_cpuus = diskann::get_mean_stats(
      stats, query_num,
      [](const diskann::QueryStats& stats) { return stats.cpu_us; });
  /*
      float recall = 0;
      if (calc_recall_flag) {
        recall = diskann::calculate_recall(query_num, gt_ids, gt_dists, gt_dim,
                                           query_result_ids[0].data(),
                                           recall_at, recall_at);
      }*/

  diskann::cout << std::setw(6) << L << std::setw(12) << optimized_beamwidth
                << std::setw(16) << qps << std::setw(16) << mean_latency
                << std::setw(16) << latency_999 << std::setw(16) << mean_ios
                << std::setw(16) << mean_cpuus << std::endl;



  diskann::cout << "Done searching. " << std::endl;


  diskann::aligned_free(query);
  if (warmup != nullptr)
    diskann::aligned_free(warmup);
  return 0;
}
using namespace std;
using namespace boost;
namespace po = boost::program_options;
int main (int argc, char *argv[]) {
  string                  data_file;
  string                  index_file;
  string                  query_file;
  int                     L;
  int                     K;
  float                   B,S;
  unsigned                num_threads,pq_disk_bytes;
  bool                    mode;
  _u64        num_nodes_to_cache;
  _u32        beamwidth;

      po::options_description desc_visible("General options");
  desc_visible.add_options()("help,h", "produce help message.")(
      "dataset", po::value(&data_file), "data path")(
      "query", po::value(&query_file), "query path")(
      "index", po::value(&index_file), "index path")(
      "K", po::value(&K)->default_value(100), "number of nearest neighbor")(
      "L", po::value(&L)->default_value(200), "number of nearest neighbor")(
      "build_memory_limit", po::value(&B), " Limit on the memory(GiB) allowed for building the index. ")(
      "search_memory_limit", po::value(&S), " Limit on the memory(GiB) allowed for searching the index. ")(
      "mode", po::value(&mode), "mode : 0 build index, 1 search")(
      "beamwidth", po::value(&beamwidth)->default_value(4), "maximum number of IO requests each query will issue per iteration of search code. Larger beamwidth williult in fewer IO round-trips per query, but might result in slightly higher number of IO requests to SSD per query. Specifying 0 will optimize the beamwidth depending on the number of threads performing search.")(
      "num_cached", po::value(&num_nodes_to_cache), "cache a num_cached nodes (which are closest to the starting point) in memory");

  po::options_description desc_hidden("Expert options");
  desc_hidden.add_options()("num_threads",
                            po::value(&num_threads)->default_value(1),
                            "Nomber of threads for fast building")
      ("pq_disk_bytes",po::value(&pq_disk_bytes)->default_value(0),"PQ_disk_bytes for very large dimensionality, use 0 for full vectors")
      ;
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
    string params = to_string(K) + " " + to_string(L) + " " + to_string(S) + " " + to_string(B) + " " + to_string(num_threads); + " " + to_string(pq_disk_bytes);
    build_index<float>(data_file.c_str(),index_file.c_str(), params.c_str(), metric);
  }else if(mode==1){

search_disk_index<float>(  index_file,
                    num_nodes_to_cache,
                   num_threads ,
                   beamwidth,
                  query_file,
                  K ,
                  L );
  }

}