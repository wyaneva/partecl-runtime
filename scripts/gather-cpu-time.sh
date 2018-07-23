#!/bin/bash

BMRK=$1
FSM_FILENAME=$2

who >> $BMRK
./../build/openmp-run.sh 256 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-256.timing"
./../build/openmp-run.sh 512 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-512.timing"
./../build/openmp-run.sh 1024 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-1024.timing"
./../build/openmp-run.sh 2048 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-2048.timing"
./../build/openmp-run.sh 4096 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-4096.timing"
./../build/openmp-run.sh 8192 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-8192.timing"
./../build/openmp-run.sh 16384 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-16384.timing"
./../build/openmp-run.sh 32768 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-32768.timing"
./../build/openmp-run.sh 65536 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-65536.timing"
./../build/openmp-run.sh 131072 Y N 100 1 $FSM_FILENAME N >> "$BMRK-cpu-131072.timing"
