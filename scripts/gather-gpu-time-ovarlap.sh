#!/bin/bash

who >> $1
./../build/gpu-test 131072 -results N -time Y -runs 100 -ldim $2 -chunks $3 >> "$1-gpu-131072-$3.timing"
