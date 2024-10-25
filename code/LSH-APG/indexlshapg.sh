#!/bin/bash


/Release/bin/LSHAPG --dataset $1 --dataset-size $2 --timeseries-size $3 --index-path $4 --K $5 --L $6 --nhashtab $7 --lowdim $8 --mode $9 &> ${10} 
