#! /usr/bin/env python

import numpy
import os.path
import sys

## experiment 4: survival #####################################################
#
# in (ttr, survive):
# - when: moment of change (decay, grow, replicate, or end)
# - what: dpe, tick, and type of change
#
# in (emerge):
# - when: after 25000 ticks
# - what: dpe, estimated cluster counts, dbscan cluster counts
#
# out (ttr):
# - column 1: dpe
# - column 2: tick where replication occurred (estimated)
# - column 3: tick where replication occurred (dbscan)
#
# out (survive):
# - column 1: dpe
# - column 2: tick where change occurred (estimated)
# - column 3: count of occurrence (estimated)
# - column 4: tick where change occurred (dbscan)
# - column 5: count of occurrence (dbscan)
#
# out (emerge):
# - column 1: dpe
# - column 2: number of clusters (estimated)
# - column 3: number of clusters (dbscan)
#
# for (ttr):
# - dot plot
#
# for (survive, emerge):
# - heat map


## read #######################################################################

def usage():
  print('usage: %s (ttr|survive|emerge) DATA_FILES' % sys.argv[0])

if 3 > len(sys.argv):
  usage()
  exit(-1)

which = sys.argv[1]
inputs = sys.argv[2:]

if which not in ['ttr', 'survive', 'emerge']:
  print('unknown action: %s' % which)
  usage()
  exit(-1)

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

if which in ['ttr', 'survive']:
  for line in lines:
    for item in line.split(': ')[1].split('; '):
      words = item.split()
      dpe = float(words[0])
      if dpe not in data:
        data[dpe] = {
          'est_when': [],
          'est_how': [],
          'dbscan_when': [],
          'dbscan_how': []
        }
      data[dpe]['est_when'].append(int(words[1]))
      data[dpe]['est_how'].append(words[2][0])
      data[dpe]['dbscan_when'].append(int(words[4]))
      data[dpe]['dbscan_how'].append(words[5][0])

elif 'emerge' == which:
  for line in lines:
    moments = line.split(': ')
    dpe = float(moments[1])
    moment = moments[2].split(', ')
    words = moment[0].split(); magenta = int(words[0])
    words = moment[1].split(); blue = int(words[0])
    words = moment[2].split(); yellow = int(words[0])
    moment = moments[3].split(', ')
    words = moment[0].split(); cluster = int(words[0])
    words = moment[1].split(); cell = int(words[0])
    words = moment[2].split(); spore = int(words[0])
    if dpe not in data:
      data[dpe] = []
    data[dpe].append({
      'magenta': magenta,
      'blue': blue,
      'yellow': yellow,
      'cluster': cluster,
      'cell': cell,
      'spore': spore
    })


## refine #####################################################################

refined = []

if 'ttr' == which:
  for dpe, moment in data.items():
    est_len = len(moment['est_how'])
    dbscan_len = len(moment['dbscan_how'])
    for i in range(max(est_len, dbscan_len)):
      datum = [dpe, 0, 0]
      if est_len > i and 'r' == moment['est_how'][i]:
        datum[1] = moment['est_when'][i]
      if dbscan_len > i and 'r' == moment['dbscan_how'][i]:
        datum[2] = moment['dbscan_when'][i]
      refined.append(datum)

elif 'survive' == which:
  bins = numpy.arange(0, 25000, 834)
  numpy.append(bins, [25001])
  for dpe, moment in data.items():
    est_len = len(moment['est_how'])
    dbscan_len = len(moment['dbscan_how'])
    datum = [dpe, [], []]
    occur = {}
    whens = []
    for i in range(est_len):
      when = moment['est_when'][i]
# begin data munging: only use recorded tick when decayed (for cells)
      if 'd' != moment['est_how'][i]:
        when = 25000
# end data munging
      whens.append(when)
    discretised = numpy.digitize(numpy.array(whens), bins)
    for n in range(discretised.size):
      bin = bins[discretised[n] - 1]
      if bin not in occur:
        occur[bin] = 0
      occur[bin] += 1
    for when, occurrences in occur.items():
      datum[1].append([when / 25000, occurrences])
    occur = {}
    for i in range(dbscan_len):
      when = moment['dbscan_when'][i]
      if when not in occur:
        occur[when] = 0
      occur[when] += 1
    for when, occurrences in occur.items():
      datum[2].append([when / 25000, occurrences])
    refined.append(datum)

elif 'emerge' == which:
  avg_cell_size = 48
  avg_spore_size = 18
  for dpe, moments in data.items():
    for moment in moments:
      refined.append([
        dpe,
        ((moment['blue'] + moment['yellow']) / avg_cell_size)
          + (moment['magenta'] / avg_spore_size),
        moment['cluster']
      ])


## print ######################################################################

if 'ttr' == which:
  for datum in refined:
    print('{} {} {}'.format(datum[0], datum[1], datum[2]))

elif 'survive' == which:
  for datum in refined:
    est_len = len(datum[1])
    dbscan_len = len(datum[2])
    for i in range(max(est_len, dbscan_len)):
      d = [datum[0], 0, 0, 0, 0]
      if est_len > i:
        d[1] = datum[1][i][0]
        d[2] = datum[1][i][1]
      if dbscan_len > i:
        d[3] = datum[2][i][0]
        d[4] = datum[2][i][1]
      print('{} {} {} {} {}'.format(d[0], d[1], d[2], d[3], d[4]))

elif 'emerge' == which:
  for datum in refined:
    print('{} {} {}'.format(datum[0], datum[1], datum[2]))

exit(0)

