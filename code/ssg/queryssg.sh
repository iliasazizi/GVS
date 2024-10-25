#!/bin/bash
Release/tests/nssg --dataset $1 --dataset-size $2 --queries $3 --queries-size $4  --index-path $5 --timeseries-size $6 --K $7 --L $8 --mode 1 &> $9 
