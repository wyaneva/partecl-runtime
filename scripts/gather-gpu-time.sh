#!/bin/bash

BMRK=$1
FSM_FILENAME=$2

who >> $BMRK
./../build/gpu-test 256 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-256.timing"
./../build/gpu-test 512 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-512.timing"
./../build/gpu-test 1024 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-1024.timing"
./../build/gpu-test 2048 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-2048.timing"
./../build/gpu-test 4096 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-4096.timing"
./../build/gpu-test 8192 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-8192.timing"
./../build/gpu-test 16384 -results N -time Y -runs 100 -filename $FSM_FILENAME > "$BMRK-gpu-16384.timing"
./../build/gpu-test 32768 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-32768.timing"
./../build/gpu-test 65536 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-65536.timing"
./../build/gpu-test 131072 -results N -time Y -runs 100 -filename $FSM_FILENAME >> "$BMRK-gpu-131072.timing"
