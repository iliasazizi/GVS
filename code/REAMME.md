# Parameters

We list the parameters for each method in the following tables. For datasets of similar hardness, we find that similar indexing parameters work in both cases. We made changes to the indexing parameters usually due to hardness changes and the size of the dataset.

Our choice of parameters is made based on the performance at high recall (0.99). For the influence of the parameters on graph search, please check our analysis in [Supplementary Material/Parameter Tuning in Graph-Based Approximate Nearest Neighbor Methods.pdf](Supplementary_Material/Parameter_Tuning_in_Graph-Based_Approximate_Nearest_Neighbor_Methods.pdf).

## Deep

| Method (Parameters)       | Parameters                        | 1M                    | 25GB                   | 100GB             | 1B                |
|---------------------------|-----------------------------------|-----------------------|------------------------|-------------------|-------------------|
| **HNSW (M, EFC)**         | M, EFC                            | 30, 400               | 30, 600                | 50, 800           | 40, 1000          |
| **VAMANA (R, L, C, α)**   | R, L, C, α                        | 60, 128, 400, 1.2     | 60, 128, 600, 1.2      | 60, 128, 800, 1.2 | 60, 128, 800, 1.2 |
| **NSG (R, L, C)**         | R, L, C                           | 60, 128, 400          | 60, 128, 600           |                   |                   |
| **ELPIS**                 | M, EFC, LS                        | 20, 300, 0.06         | 24, 300, 0.06          | 30, 400, 0.06     | 40, 600, 0.06     |
| **SSG**                   | R, L, A                           | 60, 400, 60           | 60, 600, 60            |                   |                   |
| **HCNNG**                 | R, NC, SC                         | 3, 30, 500            | 3, 30, 800             |                   |                   |
| **NGT**                   | e, Loutdeg, S, Moutdeg            | 0.1, 10, 20, 40       | 0.1, 10, 30, 60        |                   |                   |
| **SPTAG**                 | NTree, indeg, TPTnum, TPTLS, ref  | 6, 64, 50, 2K, 6      | 6, 64, 50, 3K, 6       |                   |                   |
| **LSHAPG**                | M, EFC, NH, LD                    | 30, 400, 2, 16        | 30, 600, 2, 16         |                   |                   |
| **EFANNA/KGRAPH**         | K, mlevel, Ntrees, L, R, S        | 64, 6, 6, 256, 256, 8 | 80, 8, 8, 256, 256, 10 |                   |                   |
| **DPG**                   | K, L, Iter, R, S                  | 40, 80, 12, 60, 20    | 60, 100, 12, 60, 20    |                   |                   |

## Sift

| Method (Parameters)       | Parameters                        | 1M                    | 25GB                   | 100GB             | 1B                |
|---------------------------|-----------------------------------|-----------------------|------------------------|-------------------|-------------------|
| **HNSW (M, EFC)**         | M, EFC                            | 30, 400               | 30, 600                | 50, 800           | 40, 1000          |
| **VAMANA (R, L, C, α)**   | R, L, C, α                        | 60, 128, 400, 1.2     | 60, 128, 600, 1.2      | 60, 128, 800, 1.2 | 60, 128, 800, 1.2 |
| **NSG (R, L, C)**         | R, L, C                           | 60, 128, 400          | 60, 128, 600           |                   |                   |
| **ELPIS**                 | M, EFC, LS                        | 20, 300, 0.06         | 24, 300, 0.06          | 30, 400, 0.06     | 40, 600, 0.06     |
| **SSG**                   | R, L, A                           | 60, 400, 60           | 60, 600, 60            |                   |                   |
| **HCNNG**                 | R, NC, SC                         | 3, 30, 500            | 3, 30, 800             |                   |                   |
| **NGT**                   | e, Loutdeg, S, Moutdeg            | 0.1, 10, 20, 40       | 0.1, 10, 30, 60        |                   |                   |
| **SPTAG**                 | NTree, indeg, TPTnum, TPTLS, ref  | 6, 64, 50, 2K, 6      | 6, 64, 50, 3K, 6       |                   |                   |
| **LSHAPG**                | M, EFC, NH, LD                    | 30, 400, 2, 16        | 30, 600, 2, 16         |                   |                   |
| **EFANNA/KGRAPH**         | K, mlevel, Ntrees, L, R, S        | 64, 6, 6, 256, 256, 8 | 80, 8, 8, 256, 256, 10 |                   |                   |
| **DPG**                   | K, L, Iter, R, S                  | 40, 80, 12, 60, 20    | 60, 100, 12, 60, 20    |                   |                   |

## SALD

| Method (Parameters)       | Parameters                        | 1M                    | 25GB                   | 100GB             |
|---------------------------|-----------------------------------|-----------------------|------------------------|-------------------|
| **HNSW (M, EFC)**         | M, EFC                            | 36, 480               | 36, 720                | 60, 960           |
| **VAMANA (R, L, C, α)**   | R, L, C, α                        | 72, 154, 480, 1.2     | 72, 154, 720, 1.2      | 72, 154, 960, 1.2 |
| **NSG (R, L, C)**         | R, L, C                           | 72, 154, 480          | 72, 154, 720           |                   |
| **ELPIS**                 | M, EFC, LS                        | 24, 360, 0.06         | 29, 360, 0.06          | 36, 480, 0.06     |
| **SSG**                   | R, L, A                           | 72, 480, 72           | 72, 720, 72            |                   |
| **HCNNG**                 | R, NC, SC                         | 4, 36, 600            | 4, 36, 960             |                   |
| **NGT**                   | e, Loutdeg, S, Moutdeg            | 0.1, 12, 24, 48       | 0.1, 12, 36, 72        |                   |
| **SPTAG**                 | NTree, indeg, TPTnum, TPTLS, ref  | 7, 77, 60, 2K, 7      | 7, 77, 60, 3K, 7       |                   |
| **LSHAPG**                | M, EFC, NH, LD                    | 36, 480, 2, 19        | 36, 720, 2, 19         |                   |
| **EFANNA/KGRAPH**         | K, mlevel, Ntrees, L, R, S        | 77, 7, 7, 308, 308, 9 | 96, 10, 10, 308, 308, 12 |                   |
| **DPG**                   | K, L, Iter, R, S                  | 48, 96, 14, 72, 24    | 72, 120, 14, 72, 24    |                   |

## SEISMIC

| Method (Parameters)       | Parameters                        | 1M                    | 25GB                   | 100GB             |
|---------------------------|-----------------------------------|-----------------------|------------------------|-------------------|
| **HNSW (M, EFC)**         | M, EFC                            | 60, 800               | 60, 1200               | 100, 1600         |
| **VAMANA (R, L, C, α)**   | R, L, C, α                        | 120, 256, 800, 1.2    | 120, 256, 1200, 1.2    | 120, 256, 1600, 1.2 |
| **NSG (R, L, C)**         | R, L, C                           | 120, 256, 800         | 120, 256, 1200         |                   |
| **ELPIS**                 | M, EFC, LS                        | 40, 600, 0.06         | 48, 600, 0.06          | 60, 800, 0.06     |
| **SSG**                   | R, L, A                           | 120, 800, 120         | 120, 1200, 120         |                   |
| **HCNNG**                 | R, NC, SC                         | 6, 60, 1000           | 6, 60, 1600            |                   |
| **NGT**                   | e, Loutdeg, S, Moutdeg            | 0.1, 20, 40, 80       | 0.1, 20, 60, 120       |                   |
| **SPTAG**                 | NTree, indeg, TPTnum, TPTLS, ref  | 12, 128, 100, 4K, 12  | 12, 128, 100, 6K, 12   |                   |
| **LSHAPG**                | M, EFC, NH, LD                    | 60, 800, 2, 32        | 60, 1200, 2, 32        |                   |
| **EFANNA/KGRAPH**         | K, mlevel, Ntrees, L, R, S        | 128, 12, 12, 512, 512, 16 | 160, 16, 16, 512, 512, 20 |                   |
| **DPG**                   | K, L, Iter, R, S                  | 80, 160, 24, 120, 40  | 120, 200, 24, 120, 40  |                   |

# ParlayANN Experiments

We use the same parameters for similar methods for optimized experiments. Please find the official code for ParlayANN at the following link: [ParlayANN GitHub Repository](https://github.com/cmuparlay/ParlayANN/tree/main).
