#!/bin/bash
DATA=pathtodata
LOGS=./logs/log
INDEX_DIR=./index


#######CREATE DIRS IF NOT EXISTING ALREADY
mkdir -p $LOGS
mkdir -p $INDEX_DIR

VERSION=RepoTest
K=10
NQ=100


DATASET=datasetname
CURR_DATASET=$DATA/$DATASET.bin
QUERIES=queriesname
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256
SIZE=1000000

M=60
EFC=128

INDEX=$INDEX_DIR/HNSW_${SIZE}_${M}_${EFC}_${DATASET}/
LOGOUT=$LOGS/HNSW_${DATASET}_${SIZE}_0_${LEN}_${M}_${EFC}_index.log
./indexgraph.sh ${CURR_DATASET} ${SIZE} $INDEX $LEN $M $EFC 0 1 0 $LOGOUT



ep=0
MODE=7
for EF in 60 90 120 150 200 300 400 600 
do
mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${M}_${EFC}/${K}
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${M}_${EFC}/${K}/HNSW_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${M}_${EFC}_${QUERIES}_${NQ}_${K}_${EF}_0_0_0_search.log
./querygraph.sh ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $EF 0 $MODE $LOG_OUT
done

done




