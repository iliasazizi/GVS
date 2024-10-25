#!/bin/bash

Release/test/KGRAPH  --dataset $1 --dataset-size $2 --timeseries-size $3 --index-path $4 --K $5 --L $6 --iters $7 --R $8 --S $9 --mode 0 &> ${10}

