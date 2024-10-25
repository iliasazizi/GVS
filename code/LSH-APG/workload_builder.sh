#!/bin/bash
DATA=PATH_to_DATA
LOGS=./logs
INDEX_DIR=./index

mkdir -p $LOGS


IS_CACHE=nocache


SIZE=1000000
VERSION=REPROSearch

#####Workload
KS=10
NQ=100

############################
########### DATASET ###########
############################

DATASET=datasetname
CURR_DATASET=$DATA/$DATASET.bin
QUERIES=querysetname
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256


K=20
L=80
NH=2
LD=16


INDEX=$INDEX_DIR/LSHAPG_${DATASET}_${SIZE}_${K}_${L}_${NH}_${LD}/

LOGOUT=$LOGS/LSHAPG_${DATASET}_${SIZE}_${LEN}_${K}_${L}_${NH}_${LD}_index.log

$PRJ_DIR/indexlshapg.sh  $CURR_DATASET $SIZE $LEN $INDEX ${K} ${L} ${NH} ${LD} 0 $LOGOUT


### Increase LS for higher recall
for LS in 30 60 90 120 150 200 250 300 400 600
do

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${L}_${NH}_${LD}/${KS}

LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${L}_${NH}_${LD}/${KS}/LSHAPG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${L}_${QUERIES}_${NQ}_${KS}_${LS}_${NH}_${LD}_${IS_CACHE}_search.log

querylshapg.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $KS $LS $LOG_OUT

done
