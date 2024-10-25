# Assess Hardness and Difficulty of the Dataset through Local Intrinsic Dimensionality and Local Relative Contrast

You can measure the hardness of the dataset alone or the query set as well, providing hints on out-of-distribution (OOD) queries.

First, build the code using release.sh; then, for a dataset and query sets:

```
CURR_DATASET=dataset
CURR_QUERIES=queries
LEN=256
SIZE=1000000                             
NQ=100                                   
```

LOGOUT=$LOGS/RCLIDD_${CURR_DATASET}_${SIZE}_${LEN}.log
$PRJ_DIR/eval.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $LEN 2 $LOGOUT    # Compute LID and RC only vs data points

LOGOUT=$LOGS/RCLIDQ_${CURR_DATASET}_${SIZE}_${LEN}.log
$PRJ_DIR/eval.sh ${CURR_DATASET} ${SIZE} ${CURR_QUERIES} ${NQ} $LEN 3 $LOGOUT  #DATA and query


## Comparing same measure on data vs data and queries gives insights on nature of the hardness, is it a queryr level or data level hardness
