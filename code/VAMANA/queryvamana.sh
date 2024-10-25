#!/bin/bash

Release/tests/vamana --dataset $1 --dataset_size $2 --query $3 --query_size $4 --index $5 --timeseries_size $6 --K $7 --L $8 --nthrds 1 --mode 1 &> $9 
