#!/bin/bash



/Release/tests/efanna --dataset $1 --dataset-size $2 --timeseries-size $3 --index-path $4 --mlevel $5 --trees $6 --iterations $7 --K $8 --L $9 --R ${10} --S ${11} --mode ${12} &> ${13}
