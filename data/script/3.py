#! /usr/bin/env python

import os.path
import sys

## experiment 3: heat map #####################################################
#
# in:
# - when: every tick for 100 ticks
# - what: color and counts of left and right neighbors of each particle
#
# out:
# - column 1: left neighbor count
# - column 2: right neighbor count
# - column 3: occurrence of left-right-counts
# - column 4: color
#
# for:
# - heat map (dot plot)


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

data = []

for line in lines:
  particles = line.split(', ')
  for i, particle in enumerate(particles):
    words = particle.split()
    if 0 == i:
      words = words[1:]
    data.append({
      'color': words[1],
      'left': int(words[2]),
      'right': int(words[3])
    })


## refine #####################################################################

refined = {}

for datum in data:
  key = "{}-{}-{}".format(datum['left'], datum['right'], datum['color'])
  if key in refined:
    refined[key]['count'] += 1
  else:
    refined[key] = {
      'left': datum['left'],
      'right': datum['right'],
      'count': 1
    }


## print ######################################################################

for datum in refined.values():
  print('{} {} {}'.format(datum['left'], datum['right'], datum['count']))

exit(0)

