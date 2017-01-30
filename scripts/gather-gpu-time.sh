#!/bin/bash

who >> $1
./../build/gpu-test 256 -results N -time Y -runs 20 >> "$1-gpu-256.timing"
./../build/gpu-test 512 -results N -time Y -runs 20 >> "$1-gpu-512.timing"
./../build/gpu-test 1024 -results N -time Y -runs 20 >> "$1-gpu-1024.timing"
./../build/gpu-test 2048 -results N -time Y -runs 20 >> "$1-gpu-2048.timing"
./../build/gpu-test 4096 -results N -time Y -runs 20 >> "$1-gpu-4096.timing"
./../build/gpu-test 8192 -results N -time Y -runs 20 >> "$1-gpu-8192.timing"
./../build/gpu-test 16384 -results N -time Y -runs 20 >> "$1-gpu-16384.timing"
./../build/gpu-test 32768 -results N -time Y -runs 20 >> "$1-gpu-32768.timing"
./../build/gpu-test 65536 -results N -time Y -runs 20 >> "$1-gpu-65536.timing"
./../build/gpu-test 131072 -results N -time Y -runs 20 >> "$1-gpu-131072.timing"
