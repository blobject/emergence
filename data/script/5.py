#! /usr/bin/env python

import os.path
import sys

## experiment 5: size & noise #################################################
#
# in (median, size, lifetime):
# - when: moment of death, decay, or (in the case of dbscan) growth
# - what: tick, type of change, cell size counts
#
# in (noise):
# - when: moment of death, decay, or (in the case of dbscan) growth for each
#         increment of noise
# - what: tick, type of change
#
# out (median, size):
# - column 1: cell size
# - column 2: absolute frequency
#
# out (lifetime):
# - column 1: tick where death (est) occurred
# - column 2: relative frequency
#
# out (noise):
# - column 1: noise
# - column 2: tick where death (est) occurred
#
# for:
# - line plot, dot plot


## read #######################################################################

def usage():
  print('usage: %s (median|size|lifetime|noise) DATA_FILES' % sys.argv[0])

if 3 > len(sys.argv):
  usage()
  exit(-1)

which = sys.argv[1]
inputs = sys.argv[2:]

if which not in ['median', 'size', 'lifetime', 'noise']:
  print('unknown action: %s' % which)
  usage()
  exit(-1)

for input in inputs:
  if not os.path.isfile(input):
    print('no file: %s' % input)
    usage()
    exit(-1)

lines = []
with open(input) as f:
  lines += f.read().splitlines()


## collect ####################################################################

data = {}


## refine #####################################################################

refined = []


## print ######################################################################

exit(0)

