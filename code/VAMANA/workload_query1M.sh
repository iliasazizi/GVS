#!/bin/bash
ROOT=/home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre
DATA=$ROOT/DATA
PRJ_DIR=$ROOT/projects/VAMANA
LOGS=$PRJ_DIR/logs/query_log
INDEX_DIR=$PRJ_DIR/index

#######CREATE DIRS IF NOT EXISTING ALREADY
mkdir -p $LOGS

IS_CACHE=nocache
NQ=100
K=10

############################
########### DEEP ###########
############################
DATASET=data_size1B_deep1b_len96_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_orig10K_deep1b_len96_znorm
CURR_QUERIES=$DATA/queries/deep/$QUERIES.bin
LEN=96
QUERIES=queries_orig10K_deep1b_len96_znorm
CURR_QUERIES=$DATA/queries/deep/$QUERIES.bin
LEN=96

DATAS=DEEP

R=60
A=1.2
L=128
C=750


VERSION=FINAL1BNEW
SIZE=1000000000

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
for Ls in 400 500 600 700 800 900 
do
mkdir -p $LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}
LOG_OUT=$LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}/VAMANA_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${R}_${L}_${QUERIES}_${NQ}_${K}_${Ls}_${A}_${C}_${IS_CACHE}_search.log
$PRJ_DIR/queryvamana.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $Ls $LOG_OUT
done

exit
############################
########### DEEP ###########
############################
DATASET=data_size1B_deep1b_len96_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_size100_deep1b_len96_znorm
CURR_QUERIES=$DATA/queries/deep/$QUERIES.bin
LEN=96

DATAS=DEEP


A=1.3
L=600
C=750


VERSION=BEAMSEXP
SIZE=8000000
R=60

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
for Ls in 10 20 30 40 50 60 70 80 100 120 130 140 150
do
mkdir -p $LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}
LOG_OUT=$LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}/VAMANA_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${R}_${L}_${QUERIES}_${NQ}_${K}_${Ls}_${A}_${C}_${IS_CACHE}_search.log
$PRJ_DIR/queryvamana.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $Ls $LOG_OUT
done


exit
VERSION=BEAMSEXPSALD
A=1.3
############################
########### SALD ###########
############################
DATASET=data_size899M_sald_len128_znorm
QUERIES=queries_size100_sald_len128_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=128
DATAS=SALD
mkdir -p $INDEX_DIR/$DATAS


SIZE=1000000
R=40
L=600
C=750


INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/

for Ls in 70 80 100 120 130 140 150 160
do
mkdir -p $LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}
LOG_OUT=$LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}/VAMANA_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${R}_${L}_${QUERIES}_${NQ}_${K}_${Ls}_${A}_${C}_${IS_CACHE}_search.log
$PRJ_DIR/queryvamana.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $Ls $LOG_OUT
done


exit

SIZE=10000
R=16
L=128
C=256
A=1.3
INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/



for Ls in 40 60 80 
do

mkdir -p $LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}

LOG_OUT=$LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}/VAMANA_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${R}_${L}_${QUERIES}_${NQ}_${K}_${Ls}_${A}_${C}_${IS_CACHE}_search.log

$PRJ_DIR/queryvamana.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $Ls $LOG_OUT

done






SIZE=100000
R=24
L=256
C=500

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/



for Ls in 40 60 80 100
do

mkdir -p $LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}

LOG_OUT=$LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}/VAMANA_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${R}_${L}_${QUERIES}_${NQ}_${K}_${Ls}_${A}_${C}_${IS_CACHE}_search.log

$PRJ_DIR/queryvamana.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $Ls $LOG_OUT

done





