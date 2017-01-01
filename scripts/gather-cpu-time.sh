#!/bin/bash

who >> $1
./../run-on-cpu/run-on-cpu 256 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 512 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 1024 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 2048 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 4096 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 8192 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 16384 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 32768 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 65536 -results N -time Y -runs 20 >> $1
./../run-on-cpu/run-on-cpu 131072 -results N -time Y -runs 20 >> $1
