# Reads the aggregated files for GPU and CPU timings and produces the file read by the plotting libraries
#
# Input GPU:
# Input CPU:
#
# Output:

import numpy
import argparse
import matplotlib.pyplot as plt 

parser = argparse.ArgumentParser()
parser.add_argument('gpufile', nargs='?', help='gpu data')
parser.add_argument('cpufile', nargs='?', help='cpu data')
parser.add_argument('bmrkname', nargs='?', help='cpu data')
args = parser.parse_args()

gpudata=numpu.loadtxt(args.gpufile, skiprows=1)
cpudata=numpu.loadtxt(args.cpufile, skiprows=1)

