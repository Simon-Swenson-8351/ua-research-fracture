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
        CHAIN_IDX_CUR=0
        while (( CHAIN_IDX_CUR < NUM_CHAINS ))
        do
            echo "./driver_inference_mh "\
                    "-d $DATA_FOLDER " \
                    "-i $DATASET_CUR " \
                    "-m $EXPLR_RATE_MULTIPLIER " \
                    "-e $EXPLR_RATE_CUR "\
                    "-c $CHAIN_IDX_CUR "\
                    "-l $CHAIN_LEN" \
                            >> $PARALLEL_FILE
            CHAIN_IDX_CUR=$(( CHAIN_IDX_CUR + 1 ))
        done
        EXPLR_RATE_CUR=$(( EXPLR_RATE_CUR + EXPLR_RATE_STRIDE ))
    done
    DATASET_CUR=$(( DATASET_CUR + 1 ))
done

cat $PARALLEL_FILE

parallel --jobs $NUM_JOBS < $PARALLEL_FILE

rm $PARALLEL_FILE


