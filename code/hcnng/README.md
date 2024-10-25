# The Source Code for HCNNG

### Introduction

This repository contains the source code for the **HCNNG** algorithm,
### Modifications

We have modified the original code by:

1. **Adding binary file support**: Implemented functionality to load raw vector data from binary files, requiring only the number of points and dimensions as input.
2. **Facilitating experiment management**: Provided a C++ interface to run indexing and search separately, simplifying the process of managing and running experiments on HNSW
3. **Improving Distance computation**: For fairness, We add efficient SIMD code for L2 distqnce, similar to the one used by HNSW and NSG
### Prerequisites

- GCC 4.9+ with OpenMP
- CMake 3.5+
- Boost 1.5+

### Compilation on Linux
```shell
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Building
```shell
indexhcnng.sh dataset n dim index mstdeg numclring sizeclstr
```
Where:
- **`mstdeg`**: MSTs max outdegree.
- **`numclring`**: Number of clustering.
- **`sizeclstr`**: max size of cluster.

---

### Suggested Parameter Values

| **Parameter** | **Description**                           | ** Values (1M Dataset)** | ** Values (25GB Dataset)** | 
|---------------|-------------------------------------------|-----------------------------------|-------------------------------------|
| **`mstdeg`**  | Mstdeg                        | 3                                 | 3                               | 
| **`numclring`** | Number of cluserting (iterations)      | 20                                 | 20                                  | 
| **`sizecltr`**       | Cluster size                   | 200                              | 5OO                                  | 

**The maximum outdegree is bounded by mstdeg*numclring

### Search
```shell
querykgraph.sh dataset n queries nq index dim K L 
```
