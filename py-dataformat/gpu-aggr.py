# Reads the data files with the GPU details and aggregates the medians
#
# Input:
# Device: Intel(R) Core(TM) i5-4570 CPU @ 3.20GHz 
# Compute units: 4 
# Work-group size: 8192 
# Local memory size (in bytes): 32768 
# Number of test cases: 1
# Time in ms
# trans-inputs trans-results exec-kernel time-total 
# 0.002968 0.000515 8.252334 8.377585 
# 0.001847 0.000866 8.280899 8.375005 
# 0.000932 0.000831 7.978128 8.085079 
# 0.002153 0.000461 8.012294 8.039907 
# 0.001867 0.000441 8.081438 8.098268 
#
# Output:
# trans-inputs trans-results exec-kernel time-total
# 0.001867 0.000515 8.081438 8.098268

import numpy
import argparse
import matplotlib.pyplot as plt 

parser = argparse.ArgumentParser()
parser.add_argument('datafiles', nargs='+', help='gpu data')
args = parser.parse_args()

print("trans-inputs trans-results exec-kernel time-total") 
for datafile in args.datafiles:
    data = numpy.loadtxt(datafile, skiprows=7)
    print(numpy.median(data[:,0]), end=" ")
    print(numpy.median(data[:,1]), end=" ")
    print(numpy.median(data[:,2]), end=" ")
    print(numpy.median(data[:,3]))

