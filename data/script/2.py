#! /usr/bin/env python

import os.path
import sys

## experiment 2: stability ####################################################
#
# in:
# - when: every 100 ticks for 10^5 or 10^6 ticks
# - what: numbers of particles by color and dbscan results
#
# out:
# - column 1: tick
# - column 2: number of cells (estimated)
# - column 3: number of spores (estimated)
# - column 4: number of cells (dbscan)
# - column 5: number of spores (dbscan)
#
# for:
# - dot or line plot


## read #######################################################################

def usage():
  print('usage: %s (count|change) DATA_FILE' % sys.argv[0])

if 3 != len(sys.argv):
  usage()
  exit(-1)

which = sys.argv[1]
input = sys.argv[2]

if which not in ['count', 'change']:
  print('unknown action: %s' % which)
  usage()
  exit(-1)

if not os.path.isfile(input):
  print('no file: %s' % input)
  usage()
  exit(-1)

with open(input) as f:
  lines = f.read().splitlines()


## collect ####################################################################

data_count = {}
data_change = []

for line in lines:
  words = line.split()
  tick = words[0][:-1]
  words.pop(0)
  if 'types' == tick:
    data_change.append([])
    for particle in line[7:].split(','):
      data_change[-1].append(particle.split()[1:])
  else:
    data_count[tick] = {
      'magenta': int(words[0]),
      'blue': int(words[2]),
      'yellow': int(words[4]),
      'cluster': int(words[8]),
      'cell': int(words[10]),
      'spore': int(words[12])
    }


## refine #####################################################################

avg_cell_size = 48
avg_spore_size = 18
refined_count = {}
refined_change = []

for tick, count in data_count.items():
  cells = count['cell']
  spores = count['spore']
  if 0 == cells:
    cells = 1
  if 0 == spores:
    spores = 1
  refined_count[tick] = {
    'estimated_cells': (count['blue'] + count['yellow']) / avg_cell_size,
    'estimated_spores': count['magenta'] / avg_spore_size,
    'dbscan_cells': count['cell'],
    'dbscan_spores': count['spore']
  }

for history in data_change:
  refined_change.append([])
  for particle in history:
    refined_change[-1].append(particle)


## print ######################################################################

if 'count' == which:
  for tick, datum in refined_count.items():
    print('{} {} {} {} {}'.format(tick,
                               datum['estimated_cells'],
                               datum['estimated_spores'],
                               datum['dbscan_cells'],
                               datum['dbscan_spores']))
  exit(0)

if 'change' == which:
  for history in refined_change:
    m_b = 0; m_y = 0; m_w = 0; m_g = 0
    b_m = 0; b_y = 0; b_w = 0; b_g = 0
    y_m = 0; y_b = 0; y_w = 0; y_g = 0
    w_m = 0; w_b = 0; w_y = 0; w_g = 0
    g_m = 0; g_b = 0; g_y = 0; g_w = 0

    for particle in history:
      for i, color in enumerate(particle):
        if 0 == i:
          continue
        prev = particle[i - 1]
        if color == prev:
          continue
        if   'm' == prev:
          if   'b' == color: m_b += 1
          elif 'y' == color: m_y += 1
          elif 'w' == color: m_w += 1
          elif 'g' == color: m_g += 1
        elif 'b' == prev:
          if   'm' == color: b_m += 1
          elif 'y' == color: b_y += 1
          elif 'w' == color: b_w += 1
          elif 'g' == color: b_g += 1
        elif 'y' == prev:
          if   'm' == color: y_m += 1
          elif 'b' == color: y_b += 1
          elif 'w' == color: y_w += 1
          elif 'g' == color: y_g += 1
        elif 'w' == prev:
          if   'm' == color: w_m += 1
          elif 'b' == color: w_b += 1
          elif 'y' == color: w_y += 1
          elif 'g' == color: w_g += 1
        elif 'g' == prev:
          if   'm' == color: g_m += 1
          elif 'b' == color: g_b += 1
          elif 'y' == color: g_y += 1
          elif 'w' == color: g_w += 1

    print("m>b: %d" % (m_b - b_m))
    print("m>y: %d" % (m_y - y_m))
    print("m>w: %d" % (m_w - w_m))
    print("m>g: %d" % (m_g - g_m))
    print("b>y: %d" % (b_y - y_b))
    print("b>w: %d" % (b_w - w_b))
    print("b>g: %d" % (b_g - g_b))
    print("y>w: %d" % (y_w - w_y))
    print("y>g: %d" % (y_g - g_y))
    print("w>g: %d" % (w_g - g_w))
    print()

  exit(0)

