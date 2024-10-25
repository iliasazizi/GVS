# The Source Code for LSH-APG

## Introduction

This repository contains the source code for the **LSH-APG** algorithm, as described in the paper:

**"Towards Efficient Index Construction and Approximate Nearest Neighbor Search in High-Dimensional Spaces" (Submitted to PVLDB 2023).**

## Modifications

We have modified the original code by:

1. **Adding binary file support**: Implemented functionality to load raw vector data from binary files, requiring only the number of points and dimensions as input.

2. **Facilitating experiment management**: Provided a C++ `main.cpp` interface to run indexing and search separately, simplifying the process of managing and running experiments on LSH-APG.

## Building

Run the script `release` to build the project using CMake.

## Indexing

To run the index, please specify:

- The dataset name and path.
- The number of points and dimensions.
- Values for the parameters **M**, **EFC**, **Number of Hash Tables (NH)**, and **Low Dimensions (LD)** in `workload_builder.sh`.

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

## Search

To run the search, please specify:

- The dataset name and query set.
- The indexing values and path to the indices folder.
- The number of nearest neighbors **K** to retrieve from the search.
- The number of queries to be answered.
- The workload information.
- The beam width size (should be greater than **K**).

Please ensure all parameters are correctly set before running the experiments.
