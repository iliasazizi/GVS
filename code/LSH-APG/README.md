# The Source Code for LSH-APG

### Introduction

This repository contains the source code for the **LSH-APG** algorithm, as described in the paper:

**"Towards Efficient Index Construction and Approximate Nearest Neighbor Search in High-Dimensional Spaces" (Submitted to PVLDB 2023).**

### Modifications

We have modified the original code by:

1. **Adding binary file support**: Implemented functionality to load raw vector data from binary files, requiring only the number of points and dimensions as input.

2. **Facilitating experiment management**: Provided a C++ `main.cpp` interface to run indexing and search separately, simplifying the process of managing and running experiments on LSH-APG.


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
./Release/bin/LSHAPG --dataset dataset.bin --dataset-size n --timeseries-size dim --index-path indexdirname --K R --L L --nhashtab nh --lowdim ld --mode 0
```

Where:
- `path/dataset.bin` is the absolute path to the dataset binary file.
- `n` is the dataset size.
- `path/indexdirname/` is the absolute path where the index will be stored (the index folder should not already exist).
- `dim` is the dimension.
- `R` is the maximum outdegree for nodes during graph construction.
- `L` is the beamwidth during candidate neighbor search.
- `nh` is the number of hash tables
- `ld` is Low dimensionality : number of hashes per table


### Parameters

Following the authors' suggestions, we use the following values:

#### For 1M Dataset

- **M** = 20
- **EFC** = 600
- **NH** = 2
- **LD** = 16

#### For 25GB Dataset

We use similar values:

- **M** = 30
- **EFC** = 600

We vary **NH** and **LD** from 2,16 to 3,32 respectively, depending on the dataset's complexity.

### Parameters Table

| **Parameter** | **Description**                           | **Values (1M Dataset)** | **Values (25GB Dataset)** |
|---------------|-------------------------------------------|--------------------------|---------------------------|
| **M**         | Maximum connections per node              | 20                       | 30                        |
| **EFC**       | Entry Filter Complexity                   | 600                      | 600                       |
| **NH**        | Number of Hash Tables                     | 2                        | 2 - 3                     |
| **LD**        | Low Dimensionality for Projection         | 16                       | 16 - 32                   |

### Search
```shell
/Release/bin/LSHAPG --dataset dataset.bin --dataset-size n --queries path/queries.bin --queries-size nq --index-path path/indexdirname/ --timeseries-size dim  --K k  --L beamwidth 
```
Where:
- `path/queries.bin` is the absolute path to the query set binary file.
- `nq` is the query set size.
- `k` is  the number of queries to be answered.
- `L` is thebeam width size (should be greater than **K**).

### Workload
To automate multiple run, please change the workload.sh with correct data path and parameters 
