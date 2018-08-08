#!/bin/bash

NUM_TESTS=$1
FSM_FILENAME=$2
NUM_CPU_THREADS=$3
DO_SORT=$4
CHUNKSIZE=$5

cd ../build
make clean && make
cd ../scripts

./../build/gpu-test $NUM_TESTS -results Y -time N -runs 1 -filename $FSM_FILENAME -chunksize $CHUNKSIZE -sort $DO_SORT > "gpu.correctness"
./../build/openmp-run.sh $NUM_TESTS N Y 1 $NUM_CPU_THREADS $FSM_FILENAME Y > "cpu.correctness"

diff "gpu.correctness" "cpu.correctness" 

#rm "gpu.correctness"
#rm "cpu.correctness"
