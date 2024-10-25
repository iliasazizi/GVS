#!/bin/bash
ROOT=/home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre
DATA=$ROOT/DATA
PRJ_DIR=$ROOT/projects/VAMANA
LOGS=$PRJ_DIR/logs/index_log
INDEX_DIR=$PRJ_DIR/index

#######CREATE DIRS IF NOT EXISTING ALREADY
mkdir -p $LOGS
mkdir -p $INDEX_DIR



IS_CACHE=nocache
DATASET=data_size200GB_t2i_len200_norm
CURR_DATASET=$DATA/$DATASET.bin
QUERIES=queries_size100K_t2i_len200_org
CURR_QUERIES=$DATA/queries/t2i/$QUERIES.bin
LEN=200
SIZE=1000000

SIZE=40000000
R=60
L=128
C=128
A=1.2

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT


VERSION=t2i40M
K=10
NQ=100
for Ls in 60 90 120 150 200 300 400 600 800 1600 2028 3020
do
mkdir -p $LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}
LOG_OUT=$LOGS/${VERSION}/${SIZE}/${DATASET}/${K}/${R}_${L}_${SIZE}/VAMANA_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${R}_${L}_${QUERIES}_${NQ}_${K}_${Ls}_${A}_${C}_${IS_CACHE}_search.log
$PRJ_DIR/queryvamana.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $INDEX $LEN $K $Ls $LOG_OUT
done

exit


DATASET=data_size1281167_imagenet_len256_org
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=query_size10K_imagenet_len256_org
CURR_QUERIES=$DATA/queries/imgnet/$QUERIES.bin
LEN=256



SIZE=1000000
R=60
L=128
C=128
A=1.2

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT


VERSION=IMAGENET1M
K=10
NQ=100
for Ls in 20 40 80 120 150 200 250 300
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

mkdir -p $INDEX_DIR/$DATAS

A=1.3

APXSP=100000

SIZE=8000000
R=60
L=600
C=750

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
#$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_outdeg.log
#$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 20 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${APXSP}_approxdiam.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $APXSP $L $C $A 22 $LOGOUT
exit
SIZE=4000000
R=50


INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_outdeg.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 20 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${APXSP}_approxdiam.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $APXSP $L $C $A 22 $LOGOUT


exit

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

A=1.3

SIZE=1200000
R=40
L=600
C=750

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT

exit
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_outdeg.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 20 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_diam.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 21 $LOGOUT

exit

SIZE=10000
R=16
L=128
C=256

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_outdeg.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 20 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_diam.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 21 $LOGOUT

SIZE=100000
R=24
L=256
C=500

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_outdeg.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 20 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_diam.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 21 $LOGOUT

SIZE=1000000
R=40
L=600
C=750

INDEX=$INDEX_DIR/$DATAS/VAMANA_${SIZE}_${R}_${L}_${C}_${A}/
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_index.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 0 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_outdeg.log
$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 20 $LOGOUT
LOGOUT=$LOGS/VAMANA_${DATASET}_${SIZE}_${LEN}_${R}_${L}_${C}_${A}_diam.log
#$PRJ_DIR/indexvamana.sh  $CURR_DATASET $SIZE $LEN $INDEX $R $L $C $A 21 $LOGOUT
