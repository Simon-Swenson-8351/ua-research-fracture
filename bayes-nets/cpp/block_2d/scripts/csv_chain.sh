#!/bin/bash

source scripts/shared.sh

PARALLEL_FILE=$(basename $0)-parallel.tmp
rm $PARALLEL_FILE

DATASET_CUR=0
while (( DATASET_CUR < NUM_DATASETS ))
do
    EXPLR_RATE_CUR=$STARTING_EXPLR_RATE
    EXPLR_RATE_MAX=$(( EXPLR_RATE_CUR + NUM_EXPLR_RATE * EXPLR_RATE_STRIDE ))
    while (( EXPLR_RATE_CUR < EXPLR_RATE_MAX ))
    do
        echo "./driver_csv_chain"\
                "-g $DATA_FOLDER"\
                "-d $CSV_CHAIN_FOLDER " \
                "-i $DATASET_CUR " \
                "-e $EXPLR_RATE_CUR "\
                "-c \"$CSV_CHAIN_INDICES\" " \
                        >> $PARALLEL_FILE
        EXPLR_RATE_CUR=$(( EXPLR_RATE_CUR + EXPLR_RATE_STRIDE ))
    done
    DATASET_CUR=$(( DATASET_CUR + 1 ))
done

cat $PARALLEL_FILE
parallel --jobs $NUM_JOBS < $PARALLEL_FILE
rm $PARALLEL_FILE
