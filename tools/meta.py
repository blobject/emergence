#! /usr/bin/env python

import math
import os.path


## read #######################################################################

input = '../meta.data'

if not os.path.isfile(input):
  print('no file: ' + input)
  exit(-1)

with open(input) as f:
  lines = f.read().splitlines()


## collect ####################################################################

data = []

i = 0
for line in lines:
  words = line.split()
  tick = words[0][:-1]
  words.pop(0)
  data.append({
    'tick': int(tick),
    'magenta': int(words[0]),
    'blue': int(words[2]),
    'yellow': int(words[4]),
    'cluster': int(words[6]),
    'cell': int(words[8]),
    'spore': int(words[10])
  })
  i += 1


## refine #####################################################################

spore_hit = 0
cell_hit = 0
spore_avgs_sum = 0
cell_avgs_sum = 0

for datum in data:
  if 0 != datum['spore']:
    spore_hit += 1
    spore_avg = datum['magenta'] / datum['spore']
    spore_avgs_sum += spore_avg
  if 0 != datum['cell']:
    cell_hit += 1
    cell_avg = (datum['blue'] + datum['yellow']) / datum['cell']
    cell_avgs_sum += cell_avg

if 0 == spore_hit:
  spore_hit += 1
if 0 == cell_hit:
  cell_hit += 1

print("avg spore size: %f" % (spore_avgs_sum / spore_hit))
print("avg cell size: %f" % (cell_avgs_sum / cell_hit))

exit(0)

