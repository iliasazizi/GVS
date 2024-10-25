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
- **`mlevel`**: Maximum level of the multi-layer graph.
- **`trees`**: Number of randomized KD trees used for clustering.
- **`iterations`**: Number of iterations for graph refinement.
- **`L`**: .Number of iterations for graph refinement.
- **`R`**: Maximum outdegree (connections) per node.
- **`K`**: Number of nearest neighbors considered during graph refinement.
- **`S`**: Size of the candidate pool for neighbor selection.

---

### Suggested Parameter Values

| **Parameter** | **Description**                           | **Suggested Values (1M Dataset)** | **Suggested Values (25GB Dataset)** |
|---------------|-------------------------------------------|-----------------------------------|-------------------------------------|
| **`mlevel`**  | Max graph level                           | 8                                 | 12                                  |
| **`trees`**   | Number of random projection trees         | 8                                 | 16                                  |
| **`iterations`** | Number of refinement iterations      | 8                                 | 10                                  |
| **`L`**       | Beamwidth during search                   | 100                              | 2OO                                  |
| **`R`**       | Maximum connections per node              | 40                               | 1OO                                  |
| **`K`**       | Nearest neighbors during refinement       | 100                                | 200                                  |
| **`S`**       | Candidate pool size                       | 10                                | 20                                  |

### Search
```shell
/Release/tests/efanna --queries path/queries.bin --queries-size nq --index-path path/indexdirname/ --timeseries-size dim  --K k  --L beamwidth 
```
Where:
- `path/queries.bin` is the absolute path to the query set binary file.
- `nq` is the query set size.
- `k` is  the number of queries to be answered.
- `L` is thebeam width size (should be greater than **K**).

### Workload
To automate multiple run, please change the workload.sh with correct data path and parameters 
