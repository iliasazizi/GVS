#!/bin/bash

Release/mainTest --queries $1 --queries-size $2 --index $3 --KNN $4 --maxcheck $5  --mode $6  --dimension $7 &> $8
