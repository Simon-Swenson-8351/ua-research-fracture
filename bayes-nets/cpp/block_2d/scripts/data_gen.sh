#!/bin/bash

source scripts/shared.sh

PARALLEL_FILE=$(basename $0)-parallel.tmp
rm $PARALLEL_FILE

DATASET_CUR=0
while (( DATASET_CUR < NUM_DATASETS ))
do
    echo "./driver_data_gen -d $DATA_FOLDER -i $DATASET_CUR" >> $PARALLEL_FILE
    DATASET_CUR=$(( DATASET_CUR + 1 ))
done
cat $PARALLEL_FILE

parallel --jobs $NUM_JOBS < $PARALLEL_FILE

rm $PARALLEL_FILE
