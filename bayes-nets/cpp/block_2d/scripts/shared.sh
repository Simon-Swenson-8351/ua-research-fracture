#!/bin/bash

LAPLACE_HOSTNAME="laplace.cs.arizona.edu"

DATE_STR=$(date +%Y%m%d)

if [[ $(hostname) == $LAPLACE_HOSTNAME ]]
then
    NUM_JOBS=24
    DATA_ROOT_FOLDER=/data/fracture/bayes-nets/block_2d
else
    NUM_JOBS=11
    DATA_ROOT_FOLDER=/home/simon/fracture-data-mountpoint/bayes-nets/block_2d/
    #DATA_ROOT_FOLDER=/home/simon/local-data/bayes-nets/block_2d/
    
fi

# generation
DATA_FOLDER="${DATA_ROOT_FOLDER}/${DATE_STR}-mh-feasibility-experiment-data"
NUM_DATASETS=16

# MH inference
EXPLR_RATE_MULTIPLIER=1.5 #for metropolis hastings
#EXPLR_RATE_MULTIPLIER=0.0001 #for gradient descent
STARTING_EXPLR_RATE=1
NUM_EXPLR_RATE=1
EXPLR_RATE_STRIDE=2
NUM_CHAINS=16
CHAIN_LEN=1000000 #for metropolis hastings
#CHAIN_LEN=10000 #for gradient descent

# aggregation
CHAIN_SAMPLE_AG_FOLDER="${DATA_ROOT_FOLDER}/${DATE_STR}-mh-feasibility-experiment-chain-samples"
CHAIN_AG_FOLDER="${DATA_ROOT_FOLDER}/${DATE_STR}-mh-feasibility-experiment-chains"
CHAIN_SAMPLES_BIN_SIZE=1
CHAIN_SAMPLES_AG="0-$((CHAIN_LEN - 1));med;;${CHAIN_SAMPLES_BIN_SIZE}"
EXPLR_RATE_AG="$STARTING_EXPLR_RATE-$((STARTING_EXPLR_RATE + (NUM_EXPLR_RATE - 1) * EXPLR_RATE_STRIDE));;;$EXPLR_RATE_STRIDE"
CHAIN_AG="0-$((NUM_CHAINS-1));mean"

# csv output

# for each standard deviation, the range of chain indices to iterate over
CSV_CHAIN_FOLDER=$DATA_FOLDER
CSV_CHAIN_INDICES="0-$((NUM_CHAINS-1))"

