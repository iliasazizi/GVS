#!/bin/bash

DATA=pathtodata
LOGS=logs/index_log
INDEX_DIR=index

#######CREATE DIRS IF NOT EXISTING ALREADY
mkdir -p $LOGS
mkdir -p $INDEX_DIR

IS_CACHE=nocache
SIZE=1000000




#############
DATASET=datasetname
CURR_DATASET=$DATA/$DATASET.bin
QUERIES=queriesname
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256
SIZE=1000000

K=10
e=0.1
KS=10
E=30
S=10

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 110 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 110 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi

sudo sync;
sudo sh -c "echo 3 >> /proc/sys/vm/drop_caches";



