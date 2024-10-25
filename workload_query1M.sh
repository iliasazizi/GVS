#!/bin/bash
ROOT=/home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre
DATA=$ROOT/DATA
PRJ_DIR=$ROOT/projects/ngt/ngt-develop
LOGS=$PRJ_DIR/logs/query_log
INDEX_DIR=$PRJ_DIR/index

#######CREATE DIRS IF NOT EXISTING ALREADY
mkdir -p $LOGS
mkdir -p $INDEX_DIR

IS_CACHE=nocache
SIZE=1000000
export LD_LIBRARY_PATH=/builder/lib/NGT/:$LD_LIBRARY_PATH

NQ=100
KS=10



DATASET=datasetname
CURR_DATASET=$DATA/$DATASET.bin
QUERIES=queriesname
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256
SIZE=1000000


K=20
e=0.1
RS=20
E=60
S=30

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.2  #0.1 0.15 0.2 0.3
do

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> foorptinrt1m.log

done




K=20
e=0.1
RS=20
E=40
S=20



############################
########### DEEP ###########
############################
DATASET=data_size1B_deep1b_len96_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_size100_deep1b_len96_znorm
CURR_QUERIES=$DATA/queries/deep/$QUERIES.bin
LEN=96

SIZE=26666667
K=20
e=0.1
RS=20
E=40
S=20

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.2  #0.1 0.15 0.2 0.3
do

/usr/bin/time -v $PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> foorptinrt25gb.log

done


exit

VERSION=FINAL25GB

############################
########## SEISMIC #########
############################
DATASET=data_size100M_seismic_len256_znorm
QUERIES=queries_size100_seismic_len256_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256
SIZE=25000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in  0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log
echo "$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT"
$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done


VERSION=25GB

K=10
e=0.1
RS=10
E=30
S=10
############################
########### SIFT ###########
############################
DATASET=data_size1B_sift1b_len128_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_orig100_sift_len128_znorm
CURR_QUERIES=$DATA/queries/sift/$QUERIES.bin
LEN=128
SIZE=50000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log
echo "$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT"
$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
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


K=20
e=0.1
RS=20
E=60
S=30

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done




K=20
e=0.1
RS=20
E=40
S=20

VERSION=FINAL25GB

############################
########## SEISMIC #########
############################
DATASET=data_size100M_seismic_len256_znorm
QUERIES=queries_size100_seismic_len256_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256
SIZE=25000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log
echo "$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT"
$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done

VERSION=100GB
############################
########### DEEP ###########
############################
DATASET=data_size1B_deep1b_len96_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_orig10K_deep1b_len96_znorm
CURR_QUERIES=$DATA/queries/deep/$QUERIES.bin
LEN=96
SIZE=266666667

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log
echo "$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT"
$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done



K=20
e=0.1
RS=10
E=30
S=10
############################
########### SIFT ###########
############################
DATASET=data_size1B_sift1b_len128_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_orig100_sift_len128_znorm
CURR_QUERIES=$DATA/queries/sift/$QUERIES.bin
LEN=128
SIZE=200000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log
echo "$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT"
$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done


exit

############################
########### DEEP ###########
############################
DATASET=data_size1B_deep1b_len96_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_orig10K_deep1b_len96_znorm
CURR_QUERIES=$DATA/queries/deep/$QUERIES.bin
LEN=96
SIZE=66666667

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done





############################
########### SALD ###########
############################
DATASET=data_size899M_sald_len128_znorm
QUERIES=queries_size100_sald_len128_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=128

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done




############################
########### SIFT ###########
############################
DATASET=data_size1B_sift1b_len128_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_orig100_sift_len128_znorm
CURR_QUERIES=$DATA/queries/sift/$QUERIES.bin
LEN=128
SIZE=50000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done


exit


############################
########### SIFT ###########
############################
DATASET=data_size1B_sift1b_len128_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
QUERIES=queries_size100_sift1b_len128_znorm
CURR_QUERIES=$DATA/queries/sift/$QUERIES.bin
LEN=128



INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}

mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/builder/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/builder/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi




INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done




############################
########### SALD ###########
############################
DATASET=data_size899M_sald_len128_znorm
QUERIES=queries_size100_sald_len128_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=128


INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done



############################
########## SEISMIC #########
############################
DATASET=data_size100M_seismic_len256_znorm
QUERIES=queries_size100_seismic_len256_znorm
CURR_DATASET=/srv/karima-data/datasets/real/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done


############################
########### GIST ###########
############################
DATASET=data_size3gb_gist1m_len960_nznorm
QUERIES=queries_size100_gist1m_len960_znorm
CURR_DATASET=$DATA/$DATASET.bin
CURR_QUERIES=$DATA/queries/gist/$QUERIES.bin
LEN=960


INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}

mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/builder/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/builder/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi


INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}/PANNG.bin

mkdir -p $LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}

for LS in 0.01 0.03 0.05 0.06 0.7 0.08 0.9 0.1 0.15 0.2  #0.1 0.15 0.2 0.3
do

 
LOG_OUT=$LOGS/${VERSION}/${DATASET}_${SIZE}_${K}_${E}_${S}/${KS}/PANNG_${DATASET}_${SIZE}_0_${LEN}_${SIZE}_${K}_${E}_${QUERIES}_${NQ}_${KS}_${LS}_${S}_${RS}_${IS_CACHE}_search.log

$PRJ_DIR/NGT/builder/bin/ngt/ngt search -n $KS -e $LS $INDEX ${CURR_QUERIES} &> $LOG_OUT
done












exit
LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/builder/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi


exit

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi



#options="1:DATASET   2:SIZE    3:INDEXPATH    4:TS_SIZE   5:LS   6:M    7:EFC   8:LOGOUT "
############################
########## SEISMIC #########
############################
DATASET=data_size100M_seismic_len256_znorm
QUERIES=queries_size100_seismic_len256_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi


############################
########### SALD ###########
############################
DATASET=data_size899M_sald_len128_znorm
QUERIES=queries_size100_sald_len128_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=128


INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi


############################
########### SIFT ###########
############################
DATASET=data_size1B_sift1b_len128_znorm
QUERIES=queries_size100_sift1b_len128_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
CURR_QUERIES=$DATA/queries/sift/$QUERIES.bin
LEN=128



INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi

############################
########### GIST ###########
############################
DATASET=data_size3gb_gist1m_len960_nznorm
CURR_DATASET=$DATA/$DATASET.bin
LEN=960

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi


###########################25B 

 

##########8#################
########### DEEP ###########
############################

DATASET=data_size1B_deep1b_len96_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
LEN=96
SIZE=66666667

K=20
e=0.1
KS=20
E=40
S=20

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi


 
#options="1:DATASET   2:SIZE    3:INDEXPATH    4:TS_SIZE   5:LS   6:M    7:EFC   8:LOGOUT "
############################
########## SEISMIC #########
############################
DATASET=data_size100M_seismic_len256_znorm
QUERIES=queries_size100_seismic_len256_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=256

SIZE=25000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi

 
############################
########### SALD ###########
############################
DATASET=data_size899M_sald_len128_znorm
QUERIES=queries_size100_sald_len128_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
CURR_QUERIES=$DATA/$QUERIES.bin
LEN=128

SIZE=50000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi

 
############################
########### SIFT ###########
############################
DATASET=data_size1B_sift1b_len128_znorm
QUERIES=queries_size100_sift1b_len128_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
CURR_QUERIES=$DATA/queries/sift/$QUERIES.bin
LEN=128

SIZE=50000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi

 

##########8#################
########### DEEP ###########
############################

DATASET=data_size1B_deep1b_len96_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
LEN=96
SIZE=266666667

K=20
e=0.1
KS=20
E=40
S=20

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi

 
############################
########### SIFT ###########
############################
DATASET=data_size1B_sift1b_len128_znorm
QUERIES=queries_size100_sift1b_len128_znorm
CURR_DATASET=/data/ilias.azizi/fulldatasets/$DATASET.bin
CURR_QUERIES=$DATA/queries/sift/$QUERIES.bin
LEN=128

SIZE=200000000

INDEX=$INDEX_DIR/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}
mkdir -p $INDEX

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i 'g|t' -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -p 80 -n $SIZE $INDEX/PANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_pruning.log
TIMELOG=$INDEX/PANNG_${DATASET}_${SIZE}_${K}_${E}_${S}_time.log

{ time $PRJ_DIR/NGT/build/bin/ngt/ngt prune -e $E -s $S -p 80 $INDEX/PANNG.bin &> $LOGOUT; } 2> $TIMELOG

if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi


exit







K=30
e=0.1
KS=40

INDEX=$INDEX_DIR/NGT_${DATASET}_${SIZE}_${K}_${KS}/
mkdir -p $INDEX

LOGOUT=$INDEX/NGT_${DATASET}_${SIZE}_${K}_${KS}__construction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt create -i t -g a -S 0 -e $e -E $K -d $LEN -o f -D 2 -n $SIZE $INDEX/ANNG.bin $CURR_DATASET &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT construction"
  exit 1
fi

LOGOUT=$INDEX/NGT_${DATASET}_${SIZE}_${K}_${KS}__reconstruction.log
$PRJ_DIR/NGT/build/bin/ngt/ngt reconstruct-graph -m S -o 10 -i ${KS} $INDEX/ANNG.bin $INDEX/ONNG.bin &> $LOGOUT
if [ $? -ne 0 ]; then
  echo "Error during NGT reconstruction"
  exit 1
fi

