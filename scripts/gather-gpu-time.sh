#!/bin/bash

who >> $1
./../build/test-on-gpu 256 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 512 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 1024 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 2048 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 4096 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 8192 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 16384 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 32768 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 65536 -results N -time Y -runs 20 >> $1
./../build/test-on-gpu 131072 -results N -time Y -runs 20 >> $1
