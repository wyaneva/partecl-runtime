#!/bin/bash

NUM_TESTS=$1
FSM_FILENAME=$2
CHUNKSIZE=$3

cd ../build
make clean && make
cd ../scripts

./../build/gpu-test $NUM_TESTS -results Y -time N -runs 1 -filename $FSM_FILENAME -chunksize $CHUNKSIZE -sort Y > "gpu.correctness"
./../build/openmp-run.sh $NUM_TESTS N Y 1 8 $FSM_FILENAME Y > "cpu.correctness"

diff "gpu.correctness" "cpu.correctness" 

#rm "gpu.correctness"
#rm "cpu.correctness"
