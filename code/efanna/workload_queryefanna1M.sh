#!/bin/bash
ROOT=/home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre
#!/bin/bash
DATA=DATA
PRJ_DIR=/efanna
LOGS=$PRJ_DIR/logs/index_log
INDEX_DIR=$PRJ_DIR/index
#######CREATE DIRS IF NOT EXISTING ALREADY
mkdir -p $LOGS
mkdir -p $INDEX_DIR
IS_CACHE=nocache


VERSION=REPOTEST
NQ=100
K=10
IS_CACHE=nocache
DATASET=Datasetname
CURR_DATASET=$DATA/$DATASET.bin
QUERIES=queries
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256


Kb=40
Lb=100
R=100
S=8
LVL=6
ITER=6
TREE=6
INDEX=indextest

INDEX=$INDEX_DIR/EFANNA_${SIZE}_${Kb}_${Lb}_${DATASET}_${LEN}/

for L in 400 300 200 100 
do

mkdir -p $LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${Kb}_${Lb}_${SIZE}

LOG_OUT=$LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${Kb}_${Lb}_${SIZE}/EFANNA_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${Kb}_${Lb}_${QUERIES}_${NQ}_${K}_${L}_${S}_${R}_${IS_CACHE}_search.log

$PRJ_DIR/queryefanna.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $L $LOG_OUT

done


