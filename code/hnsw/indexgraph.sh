#!/bin/bash

/Release/HNSW --dataset $1 --dataset-size $2 --index-path $3 --timeseries-size $4  --M $5 --efconstruction $6 --rng $7 --prune $8 --mode $9 &> ${10}
