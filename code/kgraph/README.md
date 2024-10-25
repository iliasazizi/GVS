# The Source Code for KGRAPH

### Introduction

This repository contains the source code for the **KGRAPH** algorithm,
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
indexkgraph.sh dataset n dim index K L iterations R S 
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
| **`iterations`** | Number of refinement iterations      | 8                                 | 10                                  |
| **`L`**       | Beamwidth during search                   | 100                              | 2OO                                  |
| **`R`**       | Maximum connections per node              | 40                               | 1OO                                  |
| **`K`**       | Nearest neighbors during refinement       | 100                                | 200                                  |
| **`S`**       | Candidate pool size                       | 10                                | 20                                  |

### Search
```shell
querykgraph.sh dataset n queries nq index dim K L 
```
