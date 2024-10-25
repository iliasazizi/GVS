#!/bin/bash

/Release/HNSW --queries $1 --queries-size $2 --index-path $3 --timeseries-size $4 --k $5 --ef $6 --mode $8 --ep $7 &> $9


