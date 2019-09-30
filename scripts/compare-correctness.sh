#!/bin/bash

NUM_TESTS=$1
FSM_FILENAME=$2
NUM_CPU_THREADS=$3
DO_SORT=$4 # values Y or N
CHUNKSIZE=$5 # default is 0 = no chunking
DO_BUILD=$6 # values Y or N
output_file=compare.out

# build the code
if [ $DO_BUILD == Y ]
then
  cd ../build &> /dev/null
  make clean && make &>> $output_file
  cd ../scripts &> /dev/null
fi

echo "Build done"

# run the code
echo "Running gpu"
./../build/gpu-test $NUM_TESTS -results Y -time N -runs 1 -filename $FSM_FILENAME -chunksize $CHUNKSIZE -sort $DO_SORT > gpu.correctness 2>> $output_file
echo "Running cpu"
./../build/openmp-run.sh $NUM_TESTS N Y 1 $NUM_CPU_THREADS $FSM_FILENAME $DO_SORT > cpu.correctness 2>> $output_file

# remove control lines (ones which do not show test case results)
sed -i '/TC /!d' gpu.correctness
sed -i '/TC /!d' cpu.correctness

# check if results are different
diff gpu.correctness cpu.correctness > diff.result
if [ -s diff.result ] 
then
  echo -1
else
  echo 1
  rm gpu.correctness
  rm cpu.correctness
  rm diff.result
  rm compare.out
fi

