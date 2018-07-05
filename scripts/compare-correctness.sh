#!/bin/bash

NUM_TESTS=$1
FSM_FILENAME=$2

cd ../build
make clean && make
cd ../scripts

./../build/gpu-test $NUM_TESTS -results Y -time N -runs 1 -filename $FSM_FILENAME > "gpu.correctness"
./../build/openmp-run.sh $NUM_TESTS N Y 1 8 $FSM_FILENAME > "cpu.correctness"

diff "gpu.correctness" "cpu.correctness" 

#rm "gpu.correctness"
#rm "cpu.correctness"
