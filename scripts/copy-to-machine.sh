#!/bin/bash

LOGINID=$1
MACHINE=$2

scp -r ../kernel-gen/ ../cpu-gen/ $LOGINID@$MACHINE:~/partecl-runtime
