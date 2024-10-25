# The Source Code for ELPIS

### Introduction

This repository contains the source code for the **ELPIS** algorithm,
### Modifications

We have modified the original code by:

1. **Adding binary file support**: Implemented functionality to load raw vector data from binary files, requiring only the number of points and dimensions as input.
2. **Facilitating experiment management**: Provided a C++ `main.cpp` interface to run indexing and search separately, simplifying the process of managing and running experiments on HNSW



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
$ ./Release/ELPIS --dataset data --dataset-size size --index-path index_path --dim dim --leaf-size ls --K K  --L L --mode 0 
```

 Where: 
 
 + data: Path to dataset to be indexed.
 + size: Size of dataset to be indexed.
 + index_path: The path where ELPIS is supposed to create the index.
 + dim: The data dimension.
 + ls: The max leaf size.
 + K: Maximum Outdegree for hierarchical layers in leafgraphs(outdegree of base graph is x2).
 + L: Beamwidth used during graphs building.

   ### Parameters Table

| **Parameter** | **Description**                           | **Values (1M Dataset)** | **Values (25GB Dataset)**  | **Values (25GB Dataset)**| **Values (1B Dataset)**  |
|---------------|-------------------------------------------|--------------------------|---------------------------|--------------------------|---------------------------|
| **K**         | Maximum connections per node              | 20                       | 20                        | 30                       | 30                        |
| **L**       | Beam width during search                  | 300                      | 300                       | 600                      | 600                      |
| **ls*       | range                 | 6%                   | 6%                    | 6%                    | 6%                    |
## Search

```shell
$ ./Release/ELPIS --queries queries --queries-size size --index-path index_path --k K --L L --nprobes maxv --mode 1 

```

 + queries: Path to the queries set.
 + size: Size of queries set.
 + index_path: The path where ELPIS index can be found.
 + K: Number of nearest neighbors answers for each query.
 + L: Beamwidth used during graphs search.
 + maxv: Maximum number of leaves to search for each query. 
