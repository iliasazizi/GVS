# The Source Code for NSG

### Introduction

This repository contains the source code for the **NSG** algorithm,
### Modifications

We have modified the original code by:

1. **Adding binary file support**: Implemented functionality to load raw vector data from binary files, requiring only the number of points and dimensions as input.

2. **Facilitating experiment management**: Provided a C++ `main.cpp` interface to run indexing and search separately, simplifying the process of managing and running experiments on HNSW
3. **Disable cache warming**: We disabled the warming of the cache with queries results before search 

### Prerequisites

- GCC 4.9+ with OpenMP
- CMake 3.5+
- Boost 1.5+
- Gperf

### Compilation on Linux
```shell
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```


### Building
```shell
./Release/tests/nsg --dataset dataset --dataset-size n --timeseries-size dim --init-graph efannabaseindexpath --index-path indexpath --L L --R K --C range --mode 0
HNSW --dataset dataset.bin --dataset-size n --timeseries-size dim --index-path indexdirname --M M --efconstruction EFC --mode 0
```

Where:
- `dataset` is the absolute path to the dataset binary file.
- `n` is the dataset size.
- `efannabaseindexpath/` is the absolute path of efanna base graph.
- `indexpath` is the absolute path where the index will be stored (the index folder should not already exist).
- `dim` is the dimension.
- `K` is the maximum outdegree for nodes during graph construction.
- `L` is the beamwidth during candidate neighbor search.
- `C` is the max number of candidates neighbors to consider from visited list during search

### Parameters
We tune both parameters and selecte the ones giving the best efficiency accuracy tradeoff

### Parameters Table

| **Parameter** | **Description**                           | **Values (1M Dataset)** | **Values (25GB Dataset)**  |
|---------------|-------------------------------------------|--------------------------|---------------------------|
| **K**         | Maximum connections per node              | 40                       | 60                        | 
| **L**       | Beam width during search                  | 128                      | 256                       | 
| **C**       | Candidate set threshold                  | 256                      | 512                      | 

### Search
```shell
/Release/tests/nsg --queries path/queries.bin --queries-size nq --index-path path/indexdirname/ --timeseries-size dim  --K k  --L beamwidth 
```
Where:
- `path/queries.bin` is the absolute path to the query set binary file.
- `nq` is the query set size.
- `k` is  the number of queries to be answered.
- `L` is thebeam width size (should be greater than **K**).

### Workload
To automate multiple run, please change the workload.sh with correct data path and parameters 
