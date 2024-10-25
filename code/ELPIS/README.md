# ELPIS


# Building Instruction

## Prerequisites

 + GCC 4.9+ with OpenMP 
 + CMake 2.8+ 
 + Boost 1.55+ 

## Compile On Ubuntu

1.  Install Dependencies:

```shell
$ sudo apt-get install g++ cmake libboost-dev 
```

2. Compile ELPIS:

```shell
$ cd code; chmod 777 *; ./release.sh
```

## Run 

### Buiding

1. Index build

```shell
$ ./Release/ELPIS --dataset data --dataset-size size --index-path index_path --timeseries-size dimensions --leaf-size leaf_size --kb K  --Lb bw --mode 0 --buffer-size MaxGB
```
 + data: Path to dataset to be indexed.
 + size: Size of dataset to be indexed.
 + index_path: The path where ELPIS is supposed to create the index.
 + dimensions: The data dimension.
 + leaf_size: The max leaf size.
 + K: Maximum Outdegree for hierarchical layers in leafgraphs(outdegree of base graph is x2).
 + bw: Beamwidth used during graphs building.
 + buffer-size: Maximum memory size(GB) to by Hercules.

## Search

```shell
$ ./Release/ELPIS --queries queries --queries-size size --index-path index_path --k k --L bw --nprobes maxvl --mode 1 

```

 + queries: Path to the queries set.
 + size: Size of queries set.
 + index_path: The path where ELPIS index can be found.
 + k: Number of nearest neighbors answers for each query.
 + bw: Beamwidth used during graphs search.
 + maxvl: Maximum number of leaves to search for each query. 
