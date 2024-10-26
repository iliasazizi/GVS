 <meta name="robots" content="noindex">
 
# Graph-Based Vector Similarity Search: An Experimental Evaluation of the State-of-the-Art

## Introduction
Collections of vector data often expand to billions of vectors with thousands of dimensions, thereby escalating the complexity of analysis. Similarity search serves as the linchpin for numerous critical analytical tasks. Graph-based similarity search has recently emerged as the preferred method for analytical tasks that do not necessitate theoretical guarantees on the quality of answers.
In this survey, we undertake a thorough evaluation of twelve state-of-the-art in-memory graph-based similarity search methods :

|   SOTA Alg.   |     PAPER     |   CODE   |
|:--------:|:------------:|:--------:|
|  KGraph  |  [WWW'2011](https://dl.acm.org/doi/abs/10.1145/1963405.1963487)  |  [C++/Python](https://github.com/aaalgo/kgraph)  |
|  NSG        |    [VLDB'2019](http://www.vldb.org/pvldb/vol12/p461-fu.pdf)    | [C++](https://github.com/ZJULearning/nsg)      |
|  DPG        |    [TKDE'2019](https://ieeexplore.ieee.org/abstract/document/8681160)    | [C++](https://github.com/DBWangGroupUNSW/nns_benchmark/tree/master/algorithms/DPG) |
|  Vamana     |    [NeurIPS'2019](http://harsha-simhadri.org/pubs/DiskANN19.pdf)    |  [C++](https://github.com/microsoft/DiskANN)  |
|  EFANNA     |    [arXiv'2016](https://arxiv.org/abs/1609.07228)    | [C++](https://github.com/ZJULearning/efanna_graph) |
|  HNSW       | [TPAMI'2018](https://ieeexplore.ieee.org/abstract/document/8594636) | [C++/Python](https://github.com/nmslib/hnswlib) |
|  SPTAG-KDT  |  [ACM MM'2012](https://dl.acm.org/doi/abs/10.1145/2393347.2393378); [CVPR'2012](https://ieeexplore.ieee.org/abstract/document/6247790); [TPAMI'2014](https://ieeexplore.ieee.org/abstract/document/6549106)  | [C++](https://github.com/microsoft/SPTAG) |
|  SPTAG-BKT  | [ACM MM'2012](https://dl.acm.org/doi/abs/10.1145/2393347.2393378); [CVPR'2012](https://ieeexplore.ieee.org/abstract/document/6247790); [TPAMI'2014](https://ieeexplore.ieee.org/abstract/document/6549106) | [C++](https://github.com/microsoft/SPTAG) |
|  ELPIS      |  [VLDB'2023](https://www.vldb.org/pvldb/vol16/p1548-azizi.pdf)  | [C++](https://helios2.mi.parisdescartes.fr/~themisp/elpis/data/elpis-sourcecode.zip) |
|  NGT        |  [SISAP'22](https://github.com/yahoojapan/NGT) | [C++](https://github.com/yahoojapan/NGT) 
|  LSHAPG     |  [VLDB'23](https://www.vldb.org/pvldb/vol16/p1979-zhao.pdf) |  [C++](https://github.com/Jacyhust/LSH-APG)|
|  HVS        |  [VLDB'21](https://www.vldb.org/pvldb/vol15/p246-lu.pdf)  | [C++](https://github.com/Kejing-Lu/hvs)|
|  HCNNG      |  [PR'2019](https://www.sciencedirect.com/science/article/abs/pii/S0031320319302730)  |[C++](https://github.com/Lsyhprum/WEAVESS) |


Furthermore, we present a survey delineating the chronological evolution of these methods and evaluate their key design decisions, encompassing Seed Selection (SS) and Neighborhood Diversification (ND). This repository contains the code utilized to evaluate different design choices for SS and ND, and the indexing and search performance of the twelve SotA approaches. For each method, we include parametrization details and any changes done to the original code bases to ensure a fair comparison.

## Datasets
We use the following four real datasets covering a variety of domains from deep network embeddings, computer vision, neuroscience and seismology: (i) Deep contains 1 billion vectors of 96 dimensions extracted from the last layers of a convolutional
neural network; (ii) Sift consists of 1 billion SIFT vectors of size 128 representing image feature descriptions; (iii) SALD contains neuroscience MRI data and includes 200 million data series of size 128; (iv) Seismic  contains 100 million data series of size 256 representing earthquake recordings at seismic stations worldwide.

## ND/SS experiments
ND and SS experiments code is based on [nmslib/hnswlib code](https://github.com/nmslib/hnswlib)  for constructing insertion-based graphs with various ND approaches and executing searches using multiple SS techniques.

## Usage

### Prerequisites

- GCC 4.9+ with OpenMP
- CMake 3.5+

### Compilation on Linux
```shell
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Building
```shell
./Release/WTSS --dataset path/dataset.bin --dataset-size n --index-path path/indexdirname/ --timeseries-size dim  --K maxoutdegree  --L beamwidth --mode 0  --nd nd_type --prune prune_value --ep ep_type
```

Where:
- `path/dataset.bin` is the absolute path to the dataset binary file.
- `n` is the dataset size.
- `path/indexdirname/` is the absolute path where the index will be stored (the index folder should not already exist).
- `dim` is the dimension.
- `maxoutdegree` is the maximum outdegree for nodes during graph construction.
- `beamwidth` is the beamwidth during candidate neighbor search.
- `nd_type` is the type of ND method to use: 0 for RND, 1 for RRND, 2 for MOND and 3 for NoND.
- `prune_value` is the value used during ND. For RRND, a value between 1.3-1.5 is recommended; for MOND, 60 yields the best results.
- `ep` is the SS method to use during construction, with 0 for StackedNSW and 3 for KSREP.

#### ND Pruning Ratio
To output the ND pruning ratio during graph construction, uncomment the definition `STATSND` in `./include/PTK.h` lines 12, 13, 14.

#### Construction NDC
To output the number of distance calculations during indexing, uncomment the definition `DC_IDX` in `./include/PTK.h` lines 8, 9, 10.

### Search
```shell
./Release/WTSS --queries path/queries.bin --queries-size n --index-path path/indexdirname/ --timeseries-size dim  --K k  --L beamwidth --mode 1 --ep ep_type
```
Where:
- `path/queries.bin` is the absolute path to the query set binary file.
- `n` is the query set size.
- `k` is the number of NN results desired.
- `beamwidth` is the size of the priority queue used during beam search, with `beamwidth` >= `k`.
- `ep_type` is the type of SS method to use during search, with 0 for StackedNSW, 1 for medoid, 2 for SFREP, 3 for KSREP, and 4 for KDTrees.
