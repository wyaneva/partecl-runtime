#!/bin/bash

who >> $1
./openmp-run.sh 256 Y N 100 2 >> "$1-cpu2-256.timing"
./openmp-run.sh 512 Y N 100 2 >> "$1-cpu2-512.timing"
./openmp-run.sh 1024 Y N 100 2 >> "$1-cpu2-1024.timing"
./openmp-run.sh 2048 Y N 100 2 >> "$1-cpu2-2048.timing"
./openmp-run.sh 4096 Y N 100 2 >> "$1-cpu2-4096.timing"
./openmp-run.sh 8192 Y N 100 2 >> "$1-cpu2-8192.timing"
./openmp-run.sh 16384 Y N 100 2 >> "$1-cpu2-16384.timing"
./openmp-run.sh 32768 Y N 100 2 >> "$1-cpu2-32768.timing"
./openmp-run.sh 65536 Y N 100 2 >> "$1-cpu2-65536.timing"
./openmp-run.sh 131072 Y N 100 2 >> "$1-cpu2-131072.timing"
