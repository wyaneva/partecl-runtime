#!/bin/bash

who >> $1
./../run-on-cpu/run-on-cpu 256 -results N -time Y -runs 20 >> "$1-cpu-256.timing"
./../run-on-cpu/run-on-cpu 512 -results N -time Y -runs 20 >> "$1-cpu-512.timing"
./../run-on-cpu/run-on-cpu 1024 -results N -time Y -runs 20 >> "$1-cpu-1024.timing"
./../run-on-cpu/run-on-cpu 2048 -results N -time Y -runs 20 >> "$1-cpu-2048.timing"
./../run-on-cpu/run-on-cpu 4096 -results N -time Y -runs 20 >> "$1-cpu-4096.timing"
./../run-on-cpu/run-on-cpu 8192 -results N -time Y -runs 20 >> "$1-cpu-8192.timing"
./../run-on-cpu/run-on-cpu 16384 -results N -time Y -runs 20 >> "$1-cpu-16384.timing"
./../run-on-cpu/run-on-cpu 32768 -results N -time Y -runs 20 >> "$1-cpu-32768.timing"
./../run-on-cpu/run-on-cpu 65536 -results N -time Y -runs 20 >> "$1-cpu-65536.timing"
./../run-on-cpu/run-on-cpu 131072 -results N -time Y -runs 20 >> "$1-cpu-131072.timing"
