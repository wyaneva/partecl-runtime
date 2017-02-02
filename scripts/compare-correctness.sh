#!/bin/bash

./../build/gpu-test 131072 -results Y -time N -runs 1 -ldim 32 > "$1-gpu-131072.correctness"
./../build/openmp-run.sh 131072 N Y 1 8 > "$1-cpu-131072.correctness"

diff "$1-gpu-131072.correctness" "$1-cpu-131072.correctness" > "$1-131072-diff.correctness"
