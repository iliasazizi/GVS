    #include <iostream>
    #include <boost/format.hpp>
    #include <boost/program_options.hpp>
    #include <boost/progress.hpp>
    #include "sys/stat.h"
    #include <fstream>
    #include <ctime>
    #include <malloc.h>
    #include <omp.h>
    #include <algorithm>
    #include <cmath>
    #include <fstream>
    #include <iostream>
    #include <vector>
    #include <x86intrin.h>


using namespace std;
namespace po = boost::program_options;



float distL2(const float* a, const float* b, unsigned size) {
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
using namespace std;

class Point
{
public:
    int pointId, clusterId;
    int dimensions;
    vector<float> values;
    Point(int id,float * v, unsigned dim){
        pointId = id;
        values.insert(values.begin(),v,v+dim);
        dimensions = dim;
        clusterId=0;
    }

    int getDimensions() { return dimensions; }

    int getCluster() { return clusterId; }

    int getID() { return pointId; }

    void setCluster(int val) { clusterId = val; }

    float getVal(int pos) { return values[pos]; }
};

class Cluster
{
public:
    int clusterId;
    vector<float> centroid;
    vector<Point> points;
    Cluster(int clusterId, Point centroid)
    {
        this->clusterId = clusterId;
        for (int i = 0; i < centroid.getDimensions(); i++)
        {
            this->centroid.push_back(centroid.getVal(i));
        }
        this->addPoint(centroid);
    }

    void addPoint(Point p)
    {
        p.setCluster(this->clusterId);
        points.push_back(p);
    }

    bool removePoint(int pointId)
    {
        int size = points.size();

        for (int i = 0; i < size; i++)
        {
            if (points[i].getID() == pointId)
            {
                points.erase(points.begin() + i);
                return true;
            }
        }
        return false;
    }

    void removeAllPoints() { points.clear();points.shrink_to_fit(); }

    int getId() { return clusterId; }

    Point getPoint(int pos) { return points[pos]; }

    int getSize() { return points.size(); }

    float getCentroidByPos(int pos) { return centroid[pos]; }

    void setCentroidByPos(int pos, float val) { this->centroid[pos] = val; }
};

class KMeans
{
public:
    int K, iters, dimensions, total_points;
    vector<Cluster> clusters;
    string output_dir;

    void clearClusters()
    {
        for (int i = 0; i < K; i++)
        {
            clusters[i].removeAllPoints();
        }
    }

    int getNearestClusterId(Point point)
    {
        float min_dist;
        int NearestClusterId;

        min_dist = distL2(clusters[0].centroid.data(),point.values.data(),dimensions);
        NearestClusterId = clusters[0].getId();

        for (int i = 1; i < K; i++)
        {
            float dist;
            dist = distL2(clusters[i].centroid.data(),point.values.data(),dimensions);

            if (dist < min_dist)
            {
                min_dist = dist;
                NearestClusterId = clusters[i].getId();
            }
        }

        return NearestClusterId;
    }

    KMeans(int K, int iterations, string output_dir)
    {
        this->K = K;
        this->iters = iterations;
        this->output_dir = output_dir;
    }

    void run(vector<Point> &all_points)
    {
        total_points = all_points.size();
        dimensions = all_points[0].getDimensions();

        // Initializing Clusters
        vector<int> used_pointIds;

        for (int i = 1; i <= K; i++)
        {
            while (true)
            {
                int index = rand() % total_points;

                if (find(used_pointIds.begin(), used_pointIds.end(), index) ==
                    used_pointIds.end())
                {
                    used_pointIds.push_back(index);
                    all_points[index].setCluster(i);
                    Cluster cluster(i, all_points[index]);
                    clusters.push_back(cluster);
                    break;
                }
            }
        }
        cout << "Clusters initialized = " << clusters.size() << endl
             << endl;

        cout << "Running K-Means Clustering.." << endl;

        int iter = 1;
        while (true)
        {
            cout << "Iter - " << iter << "/" << iters << endl;
            bool done = true;

            // Add all points to their nearest cluster
#pragma omp parallel for reduction(&&: done)
            for (int i = 0; i < total_points; i++)
            {
                int currentClusterId = all_points[i].getCluster();
                int nearestClusterId = getNearestClusterId(all_points[i]);

                if (currentClusterId != nearestClusterId)
                {
                    all_points[i].setCluster(nearestClusterId);
                    done = false;
                }
            }

            // clear all existing clusters
            clearClusters();

            // reassign points to their new clusters
            for (int i = 0; i < total_points; i++)
            {
                // cluster index is ID-1
                clusters[all_points[i].getCluster() - 1].addPoint(all_points[i]);
            }

            // Recalculating the center of each cluster
            for (int i = 0; i < K; i++)
            {
                int ClusterSize = clusters[i].getSize();

                for (int j = 0; j < dimensions; j++)
                {
                    double sum = 0.0;
                    if (ClusterSize > 0)
                    {
#pragma omp parallel for reduction(+: sum)
                        for (int p = 0; p < ClusterSize; p++)
                        {
                            sum += clusters[i].getPoint(p).getVal(j);
                        }
                        clusters[i].setCentroidByPos(j, sum / ClusterSize);
                    }
                }
            }

            if (done || iter >= iters)
            {
                cout << "Clustering completed in iteration : " << iter << endl
                     << endl;
                break;
            }
            iter++;
        }

    }

    void save(vector<Point> &all_points){
        ofstream pointsFile;
        pointsFile.open(output_dir + "/" + to_string(K) + "map_vecid_clusterid.bin", std::ios::binary|std::ios::out);

        for (int i = 0; i < total_points; i++)
        {
            pointsFile.write((char *)&all_points[i].clusterId,dimensions*sizeof(int));
        }

        pointsFile.close();

        // Write cluster centers to file
        ofstream outfile(output_dir + "/" + to_string(K) + "-clusters.bin",std::ios::binary|std::ios::out);
        if (outfile.is_open())
        {
            for (int i = 0; i < K; i++)
            {
                cout << "Cluster " << clusters[i].getId() << " centroid : ";
                for (int j = 0; j < dimensions; j++)
                {
                    cout << clusters[i].getCentroidByPos(j) << " ";    // Output to console
                }
                cout << endl;
                outfile.write((char *)clusters[i].centroid.data(),dimensions*sizeof(float));
            }
            outfile.close();
        }
        else
        {
            cout << "Error: Unable to write to clusters.txt";
        }
    }
};



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




    po::options_description desc("Allowed options");
    desc.add(desc_visible);

    po::positional_options_description p;
    p.add("data", 1);
    p.add("output", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);}

    if(chdir(index_path.c_str()) == 0)
        throw std::runtime_error("The index folder is already existing, Please make sure to give an non existing path to generate index within!");
    mkdir(index_path.c_str() , 07777);

    float * data;
    load_data(data_path,data,num_points,ts_len);

    // Fetching points from file
    int pointId = 0;
    vector<Point> all_points;

    ifstream in(data_path.c_str(), ios::binary);
    if(!in.is_open()){cout<<"open file error"<<endl;exit(-1);}
    data = new float[ts_len];
    in.seekg(0, std::ios::beg);
    for(size_t i = 0; i < num_points; i++){
        in.read((char*)(data),ts_len*sizeof(float));
        Point point(i,data,ts_len);
        all_points.push_back(point);
    }
    in.close();

    cout << "\nData fetched successfully!" << endl
         << endl;

    // Return if number of clusters > number of points
    if ((int)all_points.size() < k)
    {
        cout << "Error: Number of clusters greater than number of points." << endl;
        return 1;
    }


    KMeans kmeans(k, iters, index_path);
    kmeans.run(all_points);
    //INIT ALL K LEAVES



    kmeans.save(all_points);






    return 0;
}