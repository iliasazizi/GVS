#!/bin/bash

Release/tests/nssg --dataset $1 --dataset-size $2 --timeseries-size $3 --init-graph $4 --index-path $5 --L $6 --R $7 --angle $8 --mode $9 &> ${10} 
