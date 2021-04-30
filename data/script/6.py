#! /usr/bin/env python

import os.path
import sys

## experiment 6: param sweep ##################################################
#
# in:
# - when: alpha @ +3 increments and beta @ +1 increments
# - what: dhi
#
# out:
# - column 1: alpha
# - column 2: beta
# - column 3: dhi
#
# for:
# - heat map


## read #######################################################################

def usage():
  print('usage: %s DATA_FILES' % sys.argv[0])

if 2 > len(sys.argv):
  usage()
  exit(-1)

inputs = sys.argv[1:]

for input in inputs:
  if not os.path.isfile(input):
    print('no file: %s' % input)
    usage()
    exit(-1)

lines = []
for input in inputs:
  with open(input) as f:
    lines += f.read().splitlines()


## collect ####################################################################

data = {}

for line in lines:
  words = line.split(': ')
  dhi = words[1]
  words = words[0].split(',')
  alpha = words[0].split('=')[1]
  beta = words[1].split('=')[1]
  name = '{}:{}'.format(alpha, beta)
  if name not in data:
    data[name] = {
      'alpha': int(alpha),
      'beta': int(beta),
      'dhi': []
    }
  data[name]['dhi'].append(float(dhi))


## refine #####################################################################

refined = []

for datum in data.values():
  refined.append([datum['alpha'], datum['beta'],
                 sum(datum['dhi']) / len(datum['dhi'])])


## print ######################################################################

for datum in refined:
  print('{} {} {}'.format(datum[0], datum[1], datum[2]))

exit(0)

