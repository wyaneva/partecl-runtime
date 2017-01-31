#!/bin/sh

export OMP_NUM_THREADS=$5
./../build/cpu-test $1 -time $2 -results $3 -runs $4 

