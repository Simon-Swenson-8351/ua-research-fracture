#!/bin/bash

source scripts/shared.sh

PARALLEL_FILE=$(basename $0)-parallel.tmp
rm $PARALLEL_FILE

# first, aggregate over chain samples
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
            echo "./driver_aggregator "\
                    "-d $DATA_FOLDER " \
                    "-o $CHAIN_SAMPLE_AG_FOLDER " \
                    "-i $DATASET_CUR " \
                    "-e $EXPLR_RATE_CUR " \
                    "-c $CHAIN_IDX_CUR "\
                    "-l \"$CHAIN_SAMPLES_AG\" "\
                            >> $PARALLEL_FILE
            CHAIN_IDX_CUR=$(( CHAIN_IDX_CUR + 1 ))
        done
        EXPLR_RATE_CUR=$(( EXPLR_RATE_CUR + EXPLR_RATE_STRIDE ))
    done
    DATASET_CUR=$(( DATASET_CUR + 1 ))
done

echo "First aggregation:"
cat $PARALLEL_FILE
parallel --jobs $NUM_JOBS < $PARALLEL_FILE
rm $PARALLEL_FILE

# now aggregate over chains
DATASET_CUR=0
while (( DATASET_CUR < NUM_DATASETS ))
do
    EXPLR_RATE_CUR=$STARTING_EXPLR_RATE
    EXPLR_RATE_MAX=$(( EXPLR_RATE_CUR + NUM_EXPLR_RATE * EXPLR_RATE_STRIDE ))
    while (( EXPLR_RATE_CUR < EXPLR_RATE_MAX ))
    do
        echo "./driver_aggregator "\
                "-d $CHAIN_SAMPLE_AG_FOLDER " \
                "-o $CHAIN_AG_FOLDER"\
                "-i $DATASET_CUR " \
                "-e $EXPLR_RATE_CUR "\
                "-c \"$CHAIN_AG\" "\
                        >> $PARALLEL_FILE
        EXPLR_RATE_CUR=$(( EXPLR_RATE_CUR + EXPLR_RATE_STRIDE ))
    done
    DATASET_CUR=$(( DATASET_CUR + 1 ))
done

echo "Second aggregation:"
cat $PARALLEL_FILE
parallel --jobs $NUM_JOBS < $PARALLEL_FILE
rm $PARALLEL_FILE
