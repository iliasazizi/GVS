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
