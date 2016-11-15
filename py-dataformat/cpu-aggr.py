# Reads the data files with the GPU details and aggregates the medians
#
# Input:
# Device: CPU.
# Number of test cases 1.
# Time in ms
# 25.431120 
# 24.877181 
# 25.267821 
# 25.085440 
# 24.882656 
#
# Output:
# cpu-time
# 25.08544

import numpy
import argparse
import matplotlib.pyplot as plt 

parser = argparse.ArgumentParser()
parser.add_argument('datafiles', nargs='+', help='cpu data')
args = parser.parse_args()

print("cpu-time") 
for datafile in args.datafiles:
    data = numpy.loadtxt(datafile, skiprows=3)
    print(numpy.median(data))

