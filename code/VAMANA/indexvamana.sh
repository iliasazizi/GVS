#!/bin/bash

Release/tests/vamana --dataset $1 --dataset_size $2 --timeseries_size $3 --index $4 --K $5 --L $6 --C $7 --alpha $8 --nthrds 110 --mode $9  &> ${10}




