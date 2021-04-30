#! /usr/bin/env python

import math
import os.path
import sys

## experiment 1: occupancy ####################################################
#
# in:
# - when: first tick, last tick
# - what: nearest neighbor distance of every particle
#
# out:
# - column 1: distance class (= floor of distance)
# - column 2: percentage of occurrence of distance class for first tick
# - column 3: percentage of occurrence of distance class for last tick
#
# for:
# - bar plot


## read #######################################################################

def usage():
  print('usage: %s DATA_FILE' % sys.argv[0])

if 2 != len(sys.argv):
  usage()
  exit(-1)

input = sys.argv[1]

if not os.path.isfile(input):
  print('no file: %s' % input)
  usage()
  exit(-1)

with open(input) as f:
  lines = f.read().splitlines()


## collect ####################################################################

data = {}

for line in lines:
  words = line.split()
  tick = words[0][:-1]
  words.pop(0)
  data[tick] = list(map(float, words))


## refine #####################################################################

refined = {i: [0] * len(data) for i in range(10)}

i = 0
for distances in data.values():
  for dist in distances:
    refined[math.floor(dist)][i] += 1
  i += 1

for counts in refined.values():
  i = 0
  for distances in data.values():
    counts[i] = counts[i] * 100 / len(distances)
    i += 1


## print ######################################################################

for distclass, percentages in refined.items():
  print('{} {}'.format(distclass, ' '.join(list(map(str, percentages)))))

exit(0)

