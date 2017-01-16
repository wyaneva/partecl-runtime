#!/bin/bash

./../build/test-on-gpu 131072 -results Y -time N -runs 1 > "$1-gpu-131072.correctness"
./../run-on-cpu/run-on-cpu 131072 -results Y -time N -runs 1 > "$1-cpu-131072.correctness"

diff "$1-gpu-131072.correctness" "$1-cpu-131072.correctness" > "$1-131072-diff.correctness"
