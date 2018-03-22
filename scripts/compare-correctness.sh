#!/bin/bash

NUM_TESTS=$1
BMRK=$2
FSM_FILENAME=$3

./../build/gpu-test $NUM_TESTS -results Y -time N -runs 1 -filename $FSM_FILENAME > "$BMRK-gpu-$NUM_TESTS.correctness"
./../build/openmp-run.sh $NUM_TESTS N Y 1 8 $FSM_FILENAME > "$BMRK-cpu-$NUM_TESTS.correctness"

diff "$BMRK-gpu-$NUM_TESTS.correctness" "$BMRK-cpu-$NUM_TESTS.correctness" > "$BMRK-$NUM_TESTS-diff.correctness"
