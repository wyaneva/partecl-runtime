#!/bin/sh

export OMP_NUM_THREADS=$5
./run-on-cpu $1 -time $2 -results $3 -runs $4 

