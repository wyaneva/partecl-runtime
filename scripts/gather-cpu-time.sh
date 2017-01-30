#!/bin/bash

who >> $1
./openmp-run.sh 256 Y N 20 1 >> "$1-cpu-256.timing"
./openmp-run.sh 512 Y N 20 1 >> "$1-cpu-512.timing"
./openmp-run.sh 1024 Y N 20 1 >> "$1-cpu-1024.timing"
./openmp-run.sh 2048 Y N 20 1 >> "$1-cpu-2048.timing"
./openmp-run.sh 4096 Y N 20 1 >> "$1-cpu-4096.timing"
./openmp-run.sh 8192 Y N 20 1 >> "$1-cpu-8192.timing"
./openmp-run.sh 16384 Y N 20 1 >> "$1-cpu-16384.timing"
./openmp-run.sh 32768 Y N 20 1 >> "$1-cpu-32768.timing"
./openmp-run.sh 65536 Y N 20 1 >> "$1-cpu-65536.timing"
./openmp-run.sh 131072 Y N 20 1 >> "$1-cpu-131072.timing"
