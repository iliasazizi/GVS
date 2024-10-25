#include <iostream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <malloc.h>
#include <x86intrin.h>
#include <queue>
#include <cmath>
#include "float.h"
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
///dist(a,b) = (a-b)**2
float l2distance(const float* a, const float* b, unsigned size) {
    float result = 0;

#ifdef __GNUC__
#ifdef __AVX__

    #define AVX_L2SQR(addr1, addr2, dest, tmp1, tmp2) \
      tmp1 = _mm256_loadu_ps(addr1);\
      tmp2 = _mm256_loadu_ps(addr2);\
      tmp1 = _mm256_sub_ps(tmp1, tmp2); \
      tmp1 = _mm256_mul_ps(tmp1, tmp1); \
      dest = _mm256_add_ps(dest, tmp1);

      __m256 sum;
      __m256 l0, l1;
      __m256 r0, r1;
      unsigned D = (size + 7) & ~7U;
      unsigned DR = D % 16;
      unsigned DD = D - DR;
      const float *l = a;
      const float *r = b;
      const float *e_l = l + DD;
      const float *e_r = r + DD;
      float unpack[8] __attribute__ ((aligned (32))) = {0, 0, 0, 0, 0, 0, 0, 0};

      sum = _mm256_loadu_ps(unpack);
      if(DR){AVX_L2SQR(e_l, e_r, sum, l0, r0);}

      for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16) {
      	AVX_L2SQR(l, r, sum, l0, r0);
      	AVX_L2SQR(l + 8, r + 8, sum, l1, r1);
      }
      _mm256_storeu_ps(unpack, sum);
      result = unpack[0] + unpack[1] + unpack[2] + unpack[3] + unpack[4] + unpack[5] + unpack[6] + unpack[7];

#else
#ifdef __SSE2__
#define SSE_L2SQR(addr1, addr2, dest, tmp1, tmp2) \
          tmp1 = _mm_load_ps(addr1);\
          tmp2 = _mm_load_ps(addr2);\
          tmp1 = _mm_sub_ps(tmp1, tmp2); \
          tmp1 = _mm_mul_ps(tmp1, tmp1); \
          dest = _mm_add_ps(dest, tmp1);

    __m128 sum;
    __m128 l0, l1, l2, l3;
    __m128 r0, r1, r2, r3;
    unsigned D = (size + 3) & ~3U;
    unsigned DR = D % 16;
    unsigned DD = D - DR;
    const float *l = a;
    const float *r = b;
    const float *e_l = l + DD;
    const float *e_r = r + DD;
    float unpack[4] __attribute__ ((aligned (16))) = {0, 0, 0, 0};

    sum = _mm_load_ps(unpack);
    switch (DR) {
        case 12:
        SSE_L2SQR(e_l+8, e_r+8, sum, l2, r2);
        case 8:
        SSE_L2SQR(e_l+4, e_r+4, sum, l1, r1);
        case 4:
        SSE_L2SQR(e_l, e_r, sum, l0, r0);
        default:
            break;
    }
    for (unsigned i = 0; i < DD; i += 16, l += 16, r += 16) {
        SSE_L2SQR(l, r, sum, l0, r0);
        SSE_L2SQR(l + 4, r + 4, sum, l1, r1);
        SSE_L2SQR(l + 8, r + 8, sum, l2, r2);
        SSE_L2SQR(l + 12, r + 12, sum, l3, r3);
    }
    _mm_storeu_ps(unpack, sum);
    result += unpack[0] + unpack[1] + unpack[2] + unpack[3];

//nomal distance
#else

    float diff0, diff1, diff2, diff3;
      const float* last = a + size;
      const float* unroll_group = last - 3;

      /* Process 4 items with each loop for efficiency. */
      while (a < unroll_group) {
          diff0 = a[0] - b[0];
          diff1 = a[1] - b[1];
          diff2 = a[2] - b[2];
          diff3 = a[3] - b[3];
          result += diff0 * diff0 + diff1 * diff1 + diff2 * diff2 + diff3 * diff3;
          a += 4;
          b += 4;
      }
      /* Process last 0-3 pixels.  Not needed for standard vector lengths. */
      while (a < last) {
          diff0 = *a++ - *b++;
          result += diff0 * diff0;
      }
#endif
#endif
#endif

    return result;
}


float relative_contrast(const float * data, const float * queries, unsigned int nd, unsigned int nq, unsigned int dim){
    float rc = 0.0;
    float * rcperquery = new float[nq];
    float * means  = new float[nq];
    float * mins  = new float[nq];
    {
#pragma omp parallel for
        for (unsigned int i = 0; i < nq; i++) {
            rcperquery[i] = 0.0;
            double mean = 0;
            float min = 100000;
            float dist;
            for (unsigned int j = 0; j < nd; j++) {
                dist = l2distance(data + j*dim, queries + i*dim, dim);
                if(dist == 0 )continue;
                mean += dist;
                if (dist < min)min = dist;
            }
            mean = mean / nd;
            means[i] = mean;
            mins[i] = min;
            rcperquery[i] = sqrt(mean / min);
        }
    }


    for(unsigned int i = 0;i<nq;i++) {
        std::cout << " RC Q "<<i<<":"<<rcperquery[i]<< " , [mean,min] : "<<sqrt(means[i])<<" " <<sqrt(mins[i])<<std::endl;
        rc += rcperquery[i] ;
    }
    rc = rc / nq;
    delete rcperquery;
    return rc;
}
float local_intrinsic_dim(const float * data, const float * queries, unsigned int nd, unsigned int nq, unsigned int dim, unsigned int k = 100) {
    float lid = 0.0;
    float *lidperquery = new float[nq];
    {
#pragma omp parallel for
    for (int i = 0; i < nq; i++) {
        std::priority_queue<float, std::vector<float>> pq;
        float maxdistk = 0;
        double sum = 0;
        float dist = 0.0;
        for (unsigned int j = 0; j < nd; j++) {
            dist = l2distance(data + j*dim, queries + i*dim, dim);
            pq.emplace(dist);
            if (pq.size() > k)pq.pop();
        }
        maxdistk = pq.top();
        while (!pq.empty()) {
            dist = pq.top();
            sum += log(dist / maxdistk)/2;
            pq.pop();
        }
        lidperquery[i] = -(k / sum);
    }
}
    for(unsigned int i = 0;i<nq;i++) {
        std::cout << " LID Q "<<i<<":"<<lidperquery[i]<<std::endl;
        lid += lidperquery[i] ;
    }
    lid = lid / nq;
    delete lidperquery;
    return lid;
}

/*** Metrics for DATA ***/
void RCLID(const float * data, unsigned int nd, unsigned int dim,unsigned int nq, unsigned int k = 100) {
    float lid = 0.0;
    float rc = 0.0;
    float *lidperquery = new float[nq];
    float *rcperquery = new float[nq];

    {
#pragma omp parallel for
        for(int i = 0; i < nq; i++){
            double meand = 0;
            std::priority_queue<float, std::vector<float>> pq;
            float maxdistk = 0;
            double sum = 0;
            float dist;
            for(int j=0;j<nd;j++){
                if(i==j)continue;
                dist = l2distance(data + j*dim, data + i*dim, dim);
                meand+=dist;
                pq.emplace(dist);
                if (pq.size() > k)pq.pop();
            }
            maxdistk = pq.top();
     	    meand = meand/(nd-1);//-1 because we skip when data is compared to itself
            rcperquery[i] = meand/maxdistk;

            while (!pq.empty()) {
                dist = pq.top();
                sum += log(dist / maxdistk)/2;
                pq.pop();
            }
            lidperquery[i] = -(k / sum);
        }
    }

    for(unsigned int i = 0;i<nq;i++) {
        std::cout << " Q "<<i<<", RC:"<<rcperquery[i]<<" , LID:"<<lidperquery[i]<<std::endl;
        lid += lidperquery[i] ;
        rc += rcperquery[i];
    }

//    delete dists;
    delete lidperquery;
    delete rcperquery;
    cout << "DATASET , RC:"<<rc/nq<<", LID:"<<lid/nq<<endl;
}
/*** Metrics for DATA&QUERY ***/
void RCLID(const float * data, const float * queries,
           unsigned int nd, unsigned int nq, unsigned int dim, unsigned int k = 100){
    float lid = 0.0;
    float rc = 0.0;
    float *lidperquery = new float[nq];
    float *rcperquery = new float[nq];

    {
#pragma omp parallel for
        for(int i = 0; i < nq; i++){
            float mind = 1000000;
            double meand = 0;
            std::priority_queue<float, std::vector<float>> pq;
            float dist;
            for(int j=0;j<nd;j++){
                dist = l2distance(data + j*dim, queries + i*dim, dim);
                meand+=dist;
                pq.emplace(dist);
                if (pq.size() > k)pq.pop();
            }
            meand = meand/(nd);
        

            float maxdistk = 0;
            double sum = 0;

            maxdistk = pq.top();

 	    rcperquery[i] = meand/maxdistk;
            while (!pq.empty()) {
                dist = pq.top();
                sum += log(dist / maxdistk)/2;
                pq.pop();
            }
            lidperquery[i] = -(k / sum);
        }
    }

    for(unsigned int i = 0;i<nq;i++) {
//        delete dists[i];
        std::cout << " Q "<<i<<", RC:"<<rcperquery[i]<<" , LID:"<<lidperquery[i]<<std::endl;
        lid += lidperquery[i] ;
        rc += rcperquery[i];
    }

//    delete dists;
    delete lidperquery;
    delete rcperquery;
    cout << "DATASET , RC:"<<rc/nq<<", LID:"<<lid/nq<<endl;
}


int main(int argc, char** argv) {
    string data_path;
    string query_path;

    unsigned int num_points;
    unsigned int num_query;
    unsigned int ts_len;
    unsigned short mode;

    unsigned pertosample;
/***
 * Parameters reading
 * ***/
    {
    po::options_description desc_visible("General options");
    desc_visible.add_options()
            ("help,h", "produce help message.")
            ("version,v", "print version information.")
            ("dataset", po::value(&data_path), "input path")
            ("dataset-size", po::value(&num_points), "number of data-points")
            ("queries", po::value(&query_path), "query path")
            ("queries-size", po::value(&num_query), "number of query")
            ("timeseries-size", po::value(&ts_len), "dimension")
            ("mode", po::value(&mode), "0 : Relative Contrast | 1 : Local intrinsic dimensionality @ k | 2 : RCLIDDATA | 3 : RCLID")
            ("k", po::value(&pertosample)->default_value(100),
             "percentage of sample to use, per default it's 100, i.e 100% of the dataset will be used for evaluation");

    po::options_description desc("Allowed options");
    desc.add(desc_visible);

    po::positional_options_description p;
    p.add("data", 1);
    p.add("output", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);
}

if(mode==0){
    std::cout<<"Dataset size "<<num_points<<", Queryset size "<<num_query<<", Dimension "<<ts_len<<"\nCalculating Relative Contrast..."<<std::endl;
    float* data = nullptr;float * query = nullptr;
    load_data(data_path,data,num_points,ts_len);
    load_data(query_path,query,num_query,ts_len);

    auto rc = relative_contrast(data,query,num_points,num_query,ts_len);
    std::cout << "Relative Contrast : "<<rc<<std::endl;
}else
    if(mode==1){
        std::cout<<"Dataset size "<<num_points<<", Queryset size "<<num_query<<", Dimension "<<ts_len<<"\nCalculating Local Intrinsic Dimensionality..."<<std::endl;
        float* data = nullptr;float * query = nullptr;
        load_data(data_path,data,num_points,ts_len);
        load_data(query_path,query,num_query,ts_len);

        auto lid = local_intrinsic_dim(data,query,num_points,num_query,ts_len);
        std::cout << "Local Intrinsic Dimensionality : "<<lid<<std::endl;
    }else
        if(mode == 2){
    std::cout<<"Dataset size "<<num_points<<", Dimension "<<ts_len<<"\nCalculating Dataset RC and LID..."<<std::endl;
    float* data = nullptr;float * query = nullptr;
    load_data(data_path,data,num_points,ts_len);
    RCLID(data,num_points,ts_len, num_query);
}else
    if(mode == 3){
    std::cout<<"Dataset size "<<num_points<<", Dimension "<<ts_len<<"\nCalculating Dataset and Query RC and LID..."<<std::endl;
    float* data = nullptr;float * query = nullptr;
    load_data(data_path,data,num_points,ts_len);
    load_data(query_path,query,num_query,ts_len);
    RCLID(data,query,num_points,num_query,ts_len);;
}
return 0;
}
