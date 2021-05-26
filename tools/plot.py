#! /usr/bin/env python

import math
from matplotlib import colors, pyplot as P
from mpl_toolkits.axes_grid1 import make_axes_locatable
import numpy as N
from os import path
from pprint import pprint
from scipy.integrate import odeint
from scipy.optimize import curve_fit
from statistics import median, quantiles
import sys


# usage: Print the oneliner usage help and optionally the available experiment
#        actions.

def usage(message, verbose=False):
  print('usage: {} {}'.format(sys.argv[0], message))
  if verbose:
    message = 'available actions:\n'
    for e in range(1,8):
      actions = '|'.join(exp_table[e].actions)
      if not actions:
        continue
      message += '  {}  {}\n'.format(e, actions)
    print(message, end='')


# find_median: Given a table of values and their frequencies, find the median
#              value.

def find_median(item_freqs):
  d = sorted(item_freqs, key=lambda item: item[0])
  n = sum([x[1] for x in item_freqs])
  i = n / 2
  j = (n + 1) / 2
  a = 0
  b = 0
  count = 0
  for item_freq in d:
    count += item_freq[1]
    if not a and count >= i:
      a = item_freq[0]
    if not b and count >= j:
      b = item_freq[0]
    if a and b:
      return (a + b) // 2 # only exp "5 median" uses this and requires an int


# discretise: Place data into bins, and return a dict whose keys are the
#             discretised items and whose values are 2-lists containing the bin
#             index and the count.

def discretise(bins, data):
  occur = {}
  discretised = N.digitize(data, bins)
  for i in range(discretised.size):
    b = discretised[i] - 1
    datum = bins[b]
    if datum not in occur:
      occur[datum] = [b, 0]
    occur[datum][1] += 1
  return occur


# hex_to_rgb: Convert a hex color string to a tuple of corresponding rgb.

def hex_to_rgb(c):
  c = c.strip('#')
  l = len(c)
  return tuple(int(c[i:i + l // 3], 16) for i in range(0, l, l // 3))


# rgb_to_dec: Convert an rgb color tuple to a list of corresponding floats.

def rgb_to_dec(c):
  return [x / 256 for x in c]


# make_colormap: Create a segmented colormap given a list of hex colors.

def make_colormap(cs):
  rgbs = [rgb_to_dec(hex_to_rgb(c)) for c in cs]
  floats = list(N.linspace(0, 1, len(rgbs)))
  m = {}
  for n, c in enumerate(['red', 'green', 'blue']):
    l = [[floats[i], rgbs[i][n], rgbs[i][n]] for i in range(len(floats))]
    m[c] = l
  return colors.LinearSegmentedColormap('custom', segmentdata=m, N=256)


# Exp: Experiment parent class.

class Exp:
  id = 0
  actions = []

  def __init__(self, args, read=True):
    # at least one argument
    if 1 > len(args):
      self.exp_usage()
      exit(-1)

    # consume the specified action ...
    if 0 < len(self.actions):
      self.action = args[0]
      args = args[1:]

    # ... and at least one argument again
    if 1 > len(args):
      self.exp_usage()
      exit(-1)

    # consume data files
    for f in args:
      if not path.isfile(f):
        print('no file: %s' % f)
        self.exp_usage()
        exit(-1)

    self.inputs = args
    if read: self.read_input()
    self.figure, self.axes = P.subplots()

  # read_input: Read in data files.

  def read_input(self):
    input = []
    for arg in self.inputs:
      with open(arg) as f:
        input += f.read().splitlines()
    self.input = input

  # exp_usage: Print oneliner usage help for the specific experiment.

  def exp_usage(self):
    usage('{}{}{} DATA_FILES'.format(
      self.id,
      ' ' if 0 < len(self.actions) else '',
      '|'.join(self.actions)))

  # check_action: Make sure the specified action is recognised by the
  #               specified experiment.

  def check_action(self):
    if self.action not in self.actions:
      print('unknown action: %s' % self.action)
      self.exp_usage()
      exit(-1)

  # collect: Store the raw data into memory.

  def collect(self):
    pass

  # refine: Prepare the data for plotting.

  def refine(self):
    pass

  # plot: Plot the data.

  def plot(self):
    pass


## Performance ################################################################

class Perf(Exp):

  id = 0
  actions = ['base_time',
             'base_dim',
             'base_dpe',
             'base_scope',
             'base_gui',
             'cl',
             'grid']

  def __init__(self, args):
    super().__init__(args, False)
    self.check_action()
    action = self.action

    if action in ['base_time', 'base_gui', 'cl', 'grid']:
      # slight ickiness: input files must be prepended with extra information
      # input={mode(str): [lines(str)]}
      input = {}
      for arg in self.inputs:
        with open(arg) as f:
          lines = f.read().splitlines()
          mode = lines[0]
          if mode not in input:
            input[mode] = []
          input[mode] += lines[1:]
      self.input = input
      return

    super().read_input()

  def collect(self):
    action = self.action
    input = self.input

    if action in ['base_time', 'base_gui', 'cl']:
      # data={mode(str): {time(int): [framerates(float)]}}
      data = {}

      for mode in input.keys():
        data[mode] = {}
      for mode, lines in input.items():
        for line in lines:
          time, fps = line.split(': ')
          time = int(time)
          if time not in data[mode]:
            data[mode][time] = []
          data[mode][time].append(float(fps))

      self.data = data
      return

    if 'grid' == action:
      # data={mode(str): {param(int): [framerates(float)]}}
      data = {}

      for mode in input.keys():
        data[mode] = {}
      for mode, lines in input.items():
        for line in lines:
          key, fpss = line.split(': ')
          key = int(key.split(',')[0][1:])
          if key not in data[mode]:
            data[mode][key] = []
          for fps in fpss.split():
            data[mode][key].append(float(fps))

      self.data = data
      return

    # data={key(int or float): [framerates(float)]}
    data = {}

    for line in input:
      key, fpss = line.split(': ')
      if 'base_dim' == action:
        key = int(key.split(',')[0])
      elif 'base_scope' == action:
        key = int(float(key))
      else:
        key = float(key)
      if key not in data:
        data[key] = []
      for fps in fpss.split():
        data[key].append(float(fps))

    self.data = data
    return

  def refine(self):
    action = self.action
    data = self.data

    if action in ['base_time', 'base_gui', 'cl', 'grid']:
      # refined={mode(str): {time(int): [framerates(float)]}}
      refined = {}

      for mode, datum in data.items():
        refined[mode] = {}
        for time, fpss in datum.items():
          refined[mode][time] = median(fpss)

      self.refined = refined
      return

    if action in ['base_dim', 'base_dpe', 'base_scope']:
      # refined={key(int or float): framerate(float)}
      refined = {}

      for dim, fpss in data.items():
        refined[dim] = median(fpss)

      self.refined = refined
      return

  # approximate_power: Fit a prediction curve to the framerate data.
  # - model: power function f(x) = a * exp(b * x)

  def approximate_power(self, xdata, ydata):
    # take the logarithm of everything to allow for linear regression
    xs = [math.log(x) for x in xdata]
    ys = [math.log(y) for y in ydata]

    def obj(x, a, b): return a + b * x
    def f(x, a, b): return a * b ** x

    (a, b), _ = curve_fit(obj, xs, ys)
    ea = N.exp(a)
    eb = N.exp(b)
    res = N.array([y - obj(x, a, b) for x, y in zip(xs, ys)])
    res = sum(res ** 2)
    tot = sum((ys - N.mean(ys)) ** 2)

    return {
      'a': ea,
      'b': eb,
      'r2': 1 - (res / tot),
      'x': xdata,
      'y': [f(x, ea, eb) for x in xs]
    }

  # approximate_s: Fit a prediction curve to the framerate data.
  # - model: (inverted) logistic function f(x) = a / (1 + b * c^x)

  def approximate_s(self, xs, ys):
    def obj(x, a, b, c): return a / (1 + b * c ** x)

    (a, b, c), _ = curve_fit(obj, xs, ys, [max(ys), median(ys), 1])
    res = N.array([y - obj(x, a, b, c) for x, y in zip(xs, ys)])
    res = sum(res ** 2)
    tot = sum((ys - N.mean(ys)) ** 2)

    return {
      'a': a,
      'b': b,
      'c': c,
      'r2': 1 - (res / tot),
      'x': xs,
      'y': [obj(x, a, b, c) for x in xs]
    }

  def plot(self):
    action = self.action
    data = self.refined

    cs = {'d': '#36383f', 'p': '#92959d'}

    if action in ['base_time', 'base_gui', 'cl', 'grid']:
      one = 'graphical' if 'base_time' == action else 'on'
      two = 'headless' if 'base_time' == action else 'off'
      cs = {'d': {one: '#cf554d', two: '#72a336'},
            'p': {one: '#a32c2d', two: '#4b7d08'}}

      for mode, time_fpss in data.items():
        # prepare
        xs = list(time_fpss.keys())
        if 'grid' == action:
          xs = N.arange(1, 31)
        ys = list(time_fpss.values())
        label = mode
        if 'base_gui' == action: label = 'gui '    + label
        elif     'cl' == action: label = 'opencl ' + label
        elif   'grid' == action: label = 'grid '   + label

        # plot
        P.scatter(xs, ys, label=label, color=cs['d'][mode], s=1)
        fit = self.approximate_power(xs, ys)
        label = '{:.1f}*{:.3f}^x, R^2={:.2f}'.format(
          fit['a'], fit['b'], fit['r2'])
        if 'grid' == action:
          fit = self.approximate_s(xs, ys)
          label = '{:d} / (1 + {:.2f} * {:.2f}^x), R^2={:.3f}'.format(
            int(fit['a']), fit['b'], fit['c'], fit['r2'])
        P.plot(fit['x'], fit['y'], label=label, color=cs['p'][mode])

      # periphery
      if 'grid' == action:
        P.xlabel('params [custom]')
      else:
        P.xlabel('time [sec]')
      P.ylabel('framerate [frames/sec]')
      if 'grid' == action:
        P.xlim(0, 30)
      else:
        P.xlim(0, 120)

    elif action in ['base_dim', 'base_dpe', 'base_scope']:
      # prepare
      xs = list(data.keys())
      ys = list(data.values())

      # plot
      P.scatter(xs, ys, color=cs['d'], s=1, zorder=10)
      fit = self.approximate_power(xs, ys)
      P.plot(fit['x'], fit['y'],
             label='({:.2e})*{:.3f}^x, R^2={:.2f}'
               .format(fit['a'], fit['b'], fit['r2']),
             color=cs['p'], zorder=0)

      # periphery
      P.ylabel('framerate [frames/sec]')
      if 'base_dim' == action:
        P.xlabel('habitat dimensions [su^2]')
        P.ylim(0, 1500)
      elif 'base_dpe' == action:
        P.xlabel('DPE [p/su]')
        P.ylim(0, 210)
      elif 'base_scope' == action:
        P.xlabel('neighborhood radius [su]')
        P.ylim(0, 100)

    P.rcParams['legend.handlelength'] = 0.5

    # set
    P.legend()
    P.show()


## Experiment 1: "occupancy" ##################################################
#
# plot type:
# - histogram
#
# in:
# - when: first tick, last tick
# - what: nearest neighbor distance of every particle
#
# out:
# - x:  distance class (= floor of distance)
# - y1: percentage of occurrence of distance class for first tick
# - y2: percentage of occurrence of distance class for last tick

class Exp1(Exp):

  id = 1
  actions = []

  def __init__(self, args):
    super().__init__(args)

  def collect(self):
    input = self.input

    # data={tick(int): [distances]}
    data = {}

    for line in input:
      tick, words = line.split(': ')
      data[int(tick)] = list(map(float, words.split()))

    self.data = data

  def refine(self):
    data = self.data

    # refined={tick(int): [percentages of dists(int)]}
    refined = {tick: {} for tick in data.keys()}

    # count up distances
    for tick, dists in data.items():
      for dist in dists:
        dist_class = math.floor(dist)
        if dist_class not in refined[tick]:
          refined[tick][dist_class] = 0
        refined[tick][dist_class] += 1

    # convert counts to percentages
    for tick, dist_counts in refined.items():
      refined[tick] = {dist: count / len(data[tick])
                       for dist, count in dist_counts.items()}

    self.refined = refined

  def plot(self):
    data = self.refined

    # prepare
    span = max([max(dists) for dists in data.values()]) + 2
    xs = N.arange(span)
    ys_first = [0.0] * span
    ys_last = [0.0] * span
    yss = list(data.values())
    for dist, perc in yss[0].items(): ys_first[dist] = perc
    for dist, perc in yss[1].items(): ys_last[dist] = perc
    w = 0.4

    # plot
    P.bar(xs - w / 2, ys_first, w, label='t=0',   color='#c0c0c0')
    P.bar(xs + w / 2, ys_last,  w, label='t=150', color='#000000')

    # periphery
    P.xlabel('distance to nearest particle [su]')
    P.ylabel('relative frequency')
    P.xticks(N.arange(0, 8)) # assumption
    P.yticks(N.arange(0, 1.01, 0.1))
    P.xlim(-0.5, 7.5) # assumption
    P.rcParams['legend.handlelength'] = 0.5

    # set
    P.legend()
    P.show()


## Experiment 2: "stability" ##################################################
#
# plot type:
# - line
#
# in:
# - when: every 100 ticks for 10^5 or 10^6 ticks
# - what: numbers of particles by color and dbscan results
#
# out:
# - x:  tick
# - y1: number of cells (estimated)
# - y2: number of cells (dbscan)
# - y3: number of spores (estimated)
# - y4: number of spores (dbscan)
# - y5: prediction curve for cells (estimated)
# - y6: prediction curve for cells (dbscan)
# - y7: prediction curve for spores (estimated)
# - y8: prediction curve for spores (dbscan)

class Exp2(Exp):

  id = 2
  actions = ['count', 'change']

  def __init__(self, args):
    super().__init__(args)
    self.check_action()

  def collect(self):
    input = self.input

    if 'count' == self.action:
      # data={type(str): count(int)},
      data = {}

      for line in input:
        words = line.split()
        if 'types:' == words[0]:
          continue
        tick = int(words[0][:-1])
        words.pop(0)
        data[tick] = {
          'magenta': int(words[0]),
          'blue': int(words[2]),
          'yellow': int(words[4]),
          'cluster': int(words[8]),
          'cell': int(words[10]),
          'spore': int(words[12])
        }

      self.data = data
      return

    if 'change' == self.action:
      # data=[color(str)]
      data = []

      for line in input:
        words = line.split()
        if 'types:' != words[0]:
          continue
        words.pop(0)
        data.append([])
        for particle in line[len('types: '):].split(','):
          data[-1].append(particle.split()[1:])

      self.data = data
      return

  def refine(self):
    data = self.data

    if 'count' == self.action:
      # refined={tick(int): {datum_key(str): datum_val(int or float)}}
      refined = {}
      avg_cell_size = 48
      avg_spore_size = 18

      # record counts
      for tick, count in data.items():
        cells = count['cell']
        spores = count['spore']
        if 0 == cells:
          cells = 1
        if 0 == spores:
          spores = 1
        refined[tick] = {
          'est_cells': (count['blue'] + count['yellow']) / avg_cell_size,
          'est_spores': count['magenta'] / avg_spore_size,
          'dbscan_cells': count['cell'],
          'dbscan_spores': count['spore']
        }

      self.refined = refined
      return

    if 'change' == self.action:
      # refined=[?]
      refined = []

      for history in data:
        refined.append([])
        for particle in history:
          refined[-1].append(particle)

      self.refined = refined
      return

  # approximate: Fit a prediction curve to the population data.
  # - model: Verhulst's logistic growth f(x) = A * x * (1 - x / K)

  def approximate(self, xdata, ydata):
    # initial population assumed to be 1
    init = 1

    # make the increments denser to get a better approximation
    xs = [0] * 100
    xs.extend(N.arange(0, max(xdata), 1))
    ys = []
    for y in ydata:
      for _ in range(100):
        ys.append(y)

    # objective function is the integration of the logistic growth model's
    # differential equation
    def obj(x, a, k):
      return odeint(lambda y, x: a * y * (1 - y / k), init, xs)[:, 0]

    (a, k), _  = curve_fit(obj, xs, ys)

    # the logistic growth model again
    def verhulst(y, x): return a * y * (1 - y / k)

    return {
      'a': a,
      'k': k,
      'x': xs,
      'y': odeint(verhulst, init, xs)[:, 0]
    }

  def plot(self):
    """
    if 'change' == self.action:
      for history in self.refined['change']:
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
      return
    """

    if 'count' == self.action:
      data = self.refined

      # prepare
      color_est_cell          = '#667ad3'
      color_est_cell_line     = '#3c56aa'
      color_est_spore         = '#ba59b3'
      color_est_spore_line    = '#91328c'
      color_dbscan_cell       = '#a8b8ff'
      color_dbscan_cell_line  = '#4ea494'
      color_dbscan_spore      = '#ff99f6'
      color_dbscan_spore_line = '#ba8d3b'
      ks = data.keys() # slice 'ks' and 'vs' to truncate
      vs = data.values()
      ec = [x['est_cells'] for x in vs]
      es = [x['est_spores'] for x in vs]
      dc = [x['dbscan_cells'] for x in vs]
      ds = [x['dbscan_spores'] for x in vs]
      xs = N.array(list(ks))
      xt = N.arange(min(xs), max(xs) + 1, max(xs) // 10)

      # plot (observed counts)
      P.plot(xs, ec,
             label='measured cells  (est.)',
             color=color_est_cell, zorder=30)
      P.plot(xs, dc,
             label='measured cells  (dbscan)',
             color=color_dbscan_cell, zorder=20)
      P.plot(xs, es,
             label='measured spores (est.)',
             color=color_est_spore, zorder=10)
      P.plot(xs, ds,
             label='measured spores (dbscan)',
             color=color_dbscan_spore, zorder=0)

      # plot (fits)
      fit = self.approximate(xs, ec)
      P.plot(fit['x'], fit['y'],
             label='predicted cells  (est.)   a={:.1e}, k={:.2f}'
               .format(fit['a'], fit['k']),
             color=color_est_cell_line,
             zorder=70)
      fit = self.approximate(xs, dc)
      P.plot(fit['x'], fit['y'],
             label='predicted cells  (dbscan) a={:.1e}, k={:.2f}'
               .format(fit['a'], fit['k']),
             color=color_dbscan_cell_line,
             zorder=60)
      fit = self.approximate(xs, es)
      P.plot(fit['x'], fit['y'],
             label='predicted spores (est.)   a={:.1e}, k={:.2f}'
               .format(fit['a'], fit['k']),
             color=color_est_spore_line,
             zorder=50)
      fit = self.approximate(xs, ds)
      P.plot(fit['x'], fit['y'],
             label='predicted spores (dbscan) a={:.1e}, k={:.2f}'
               .format(fit['a'], fit['k']),
             color=color_dbscan_spore_line,
             zorder=40)

      # periphery
      P.xlabel('time step')
      P.ylabel('number of structures')
      P.xticks(ticks=xt,
               labels=['0' if not x else str(x // 1000) + 'k' for x in xt])
      P.yticks(N.arange(0, 101, 10))
      #P.xlim(0, 120000) # uncomment for slice
      #P.xlim(880000, 1000000) # uncomment for slice
      self.axes.set_aspect(350)
      self.axes.margins(x=0)
      P.rcParams['legend.handlelength'] = 1

      # set
      P.legend(fontsize='small', ncol=2)
      P.show()
      return

    if 'change' == self.action:
      # TODO
      return


## Experiment 3: "heatmap" ####################################################
#
# plot type:
# - heatmap
#
# in:
# - when: every tick for 100 ticks
# - what: color and counts of left and right neighbors of each particle
#
# out:
# - x0, y0, z0: rotational grayscale background
# - x1: left neighbor count
# - y1: right neighbor count
# - z1: color proportional to count frequency

class Exp3(Exp):

  id = 3
  actions = []

  def __init__(self, args):
    super().__init__(args)

  def collect(self):
    input = self.input

    # data=[{datum_key(str): datum_val(int or str)}]
    data = []

    for line in input:
      particles = line.split(', ')
      for i, particle in enumerate(particles):
        words = particle.split()
        if 0 == i:
          # ignore the experiment instance count
          words = words[1:]
        data.append({
          'color': words[1], # unused
          'left': int(words[2]),
          'right': int(words[3])
        })

    self.data = data

  def refine(self):
    data = self.data

    # refined={coordinate(str): {'left': num_neighbors(int),
    #                            'right': num_neighbors(int),
    #                            'count': count_of_left_right_config(int)}}
    refined = {}

    for datum in data:
      key = "{}:{}".format(datum['left'], datum['right'])
      if key not in refined:
        refined[key] = {
          'left': datum['left'],
          'right': datum['right'],
          'count': 0
        }
      refined[key]['count'] += 1

    self.refined = refined

  def plot(self):
    data = self.refined
    span = 36 # assumption

    # background
    xs_back = N.arange(span)
    ys_back = N.arange(span)
    back = []
    for x in xs_back:
      column = []
      for y in ys_back:
        v = x + y
        v = v % 51 if x > y else v % 52
        v = v * -1 if x > y else v * 1
        v = v % 25 if x > y else v % 26
        column.append(v)
      back.append(column)
    P.imshow(back, cmap='binary', vmin=0, vmax=30)

    # prepare
    green   = make_colormap(['#f0ffe0', '#72a336', '#5a8b1c',
                             '#538413', '#4b7d07', '#427400',
                             '#346600', '#0f4200', '#073500'])
    brown   = make_colormap(['#f0e0e0', '#d47f3e', '#ba6827',
                             '#b16120', '#a95a19', '#9f510f',
                             '#8f4400', '#662100', '#581200'])
    magenta = make_colormap(['#e0d0e0', '#6a0067'] + ['#000000'] * 16)
    blue    = make_colormap(['#e0e8ff', '#657ad3', '#5e73cb', '#566cc3',
                             '#4c64ba', '#3c57aa', '#013583', '#002a75']
                            + ['#002a75'] * 64)
    yellow  = make_colormap(['#fff8e0', '#916814'] + ['#000000'] * 32)
    xs = [v['left'] for v in data.values()]
    ys = [v['right'] for v in data.values()]
    m = [[0 for _ in range(max(ys) + 1)] for _ in range(max(xs) + 1)]
    for v in data.values():
      m[v['left']][v['right']] = v['count'] if v['count'] else N.nan
    cs = blue
    cs.set_under('k', alpha=0)

    # plot
    P.imshow(m, cmap=cs, interpolation='none', clim=[1, 750])

    # periphery
    P.xlabel('number of left neighbors')
    P.ylabel('number of right neighbors')
    P.xticks(N.arange(0, span + 1, 5))
    P.yticks(N.arange(0, span + 1, 5))
    self.axes.invert_yaxis()
    self.axes.margins(x=0, y=0)

    # set
    #P.colorbar()
    P.show()


## Experiment 4: "survival" ###################################################
#
#           | ttr              | survive        | emerge
# ----------+------------------+----------------+------------------
# PLOT TYPE | box-and-whiskers | heatmap        | box-and-whiskers
#           | line             |                | dot
#           |                  |                | line
# ----------+------------------+----------------+------------------
#        IN |         moment of change          | after 25000 ticks
# when      |     (decay, grow, repl, end)      |
# what      |     dpe, tick, type of change     |
# ----------+-----------------------------------+------------------
# x     OUT |                         dpe
# y1        | tick of repl     | tick of change | cluster count
# y2        | prediction curve | tick frequency | prediction curve

class Exp4(Exp):

  id = 4
  actions = ['ttr', 'survive', 'emerge']

  def __init__(self, args):
    super().__init__(args)
    self.check_action()

  def collect(self):
    input = self.input

    # data={dpe(float): {datum_key(str): [datum_vals(int or str)]}}
    data = {}

    if self.action in ['ttr', 'survive']:
      for line in input:
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

      self.data = data
      return

    if 'emerge' == self.action:
      for line in input:
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

      self.data = data
      return

  def refine(self):
    data = self.data
    refined = []

    if 'ttr' == self.action:
      for dpe, datum in data.items():
        est_len = len(datum['est_how'])
        dbscan_len = len(datum['dbscan_how'])
        for i in range(max(est_len, dbscan_len)):
          moment = [dpe, 0, 0]
          if est_len > i and 'r' == datum['est_how'][i]:
            moment[1] = datum['est_when'][i]
          if dbscan_len > i and 'r' == datum['dbscan_how'][i]:
            moment[2] = datum['dbscan_when'][i]
          refined.append(moment)

      self.refined = refined
      return

    if 'survive' == self.action:
      bins = N.append(N.arange(0, 0.98, 0.03333333), [1.1])
      for dpe, datum in data.items():
        moment = {'dpe': dpe, 'est': [], 'dbscan': []}
        whens = []
        for method in ['est', 'dbscan']:
          for i in range(len(datum[method + '_how'])):
            when = datum[method + '_when'][i] / 25000
            # DATA MUNGING: only use recorded tick if decayed (for cells only)
            if 'd' != datum['est_how'][i]:
              when = 1.0
            # end DATA MUNGING
            whens.append(when)
          occur = discretise(bins, whens)
          for when, count in occur.items():
            moment[method].append([count[0], when, count[1]])
        refined.append(moment)

      self.refined = refined
      return

    if 'emerge' == self.action:
      avg_cell_size = 48
      avg_spore_size = 18
      for dpe, datum in data.items():
        for moment in datum :
          refined.append([
            dpe,
            ((moment['blue'] + moment['yellow']) / avg_cell_size)
             + (moment['magenta'] / avg_spore_size),
            moment['cluster']
         ])

      self.refined = refined
      return

  # approximate_ttr: Fit a prediction curve to the time-to-replicate data.
  # - model: power function f(x) = a * (x ^ b)

  def approximate_ttr(self, xdata, ydata):
    small = 0.00000001

    # find a reasonable starting index
    start = 0
    for i, y in enumerate(ydata):
      if not xdata[i]:
        xdata[i] = small
      if not start and y:
        start = i
      if not y:
        ydata[i] = small
    xs = xdata[start:]

    # take the logarithm of everything to allow for linear regression
    lxs = [math.log(x) for x in xs]
    lys = [math.log(y) for y in ydata[start:]]

    def obj(x, a, b): return a + b * x
    def f(x, a, b): return a * x ** b

    (a, b), _ = curve_fit(obj, lxs, lys)
    ea = math.exp(a)
    res = N.array([y - obj(x, a, b) for x, y in zip(lxs, lys)])
    res = sum(res ** 2)
    tot = sum((lys - N.mean(lys)) ** 2)

    return {
      'start': start,
      'a': ea,
      'b': b,
      'r2': 1 - (res / tot),
      'x': xs,
      'y': [f(x, ea, b) for x in xs]
    }

  # approximate_emerge: Fit a prediction curve to the emergent population data.
  # - model: linear function f(x) = a * x + b

  def approximate_emerge(self, xdata, ydata):
    threshold = 1.3

    # find a reasonable starting index
    start = 0
    for i, y in enumerate(ydata):
      if threshold < y:
        start = i
        break
    xs = xdata[start:]
    ys = ydata[start:]

    def obj(x, a, b): return a * x + b

    (a, b), _ = curve_fit(obj, xs, ys)
    res = N.array([y - obj(x, a, b) for x, y in zip(xs, ys)])
    res = sum(res ** 2)
    tot = sum((ys - N.mean(ys)) ** 2)

    return {
      'start': start,
      'a': a,
      'b': b,
      'r2': 1 - (res / tot),
      'x': xs,
      'y': [obj(x, a, b) for x in xs]
    }
    return

  def plot(self):
    data = self.refined

    if 'ttr' == self.action:
      # prepare
      ks = [x[0] for x in data]
      es = [y[1] for y in data]
      ds = [y[2] for y in data]
      vs = {}
      for i, x in enumerate(ks):
        if x not in vs:
          vs[x] = []
        vs[x].append(es[i]) # for dbscan, change 'es' to 'ds'
      xs = N.array(list(vs.keys()))
      xt = N.arange(0, max(xs) + 1, max(xs) / 10)

      # plot (observed time steps)
      bp = P.boxplot(list(vs.values()),
                     manage_ticks=False,
                     positions=list(vs.keys()),
                     widths=[0.0005] * len(vs),
                     flierprops={'marker': '+',
                                 'markersize': 1,
                                 'markeredgecolor': '#3c56aa'},
                     whiskerprops={'linestyle': 'dashed'},
                     patch_artist=True,
                     zorder=0)
      for x in bp['boxes']:
        x.set_facecolor('#667ad3')
        x.set_edgecolor('#3c56aa')
      for x in bp['caps']: x.set_color('#3c56aa')
      for x in bp['fliers']: x.set_color('#3c56aa')
      for x in bp['medians']: x.set_color('w')
      for x in bp['whiskers']: x.set_color('#667ad3')

      # plot (fit)
      fit = self.approximate_ttr(xs, [median(y) for y in vs.values()])
      P.plot(fit['x'], fit['y'], color='#a32c2d', zorder=10)

      # periphery
      P.text(0.07, 20000, 'T(DPE) = {:.1e} / DPE^{:.2f}\n   R^2 = {:.2f}'
        .format(fit['a'], -fit['b'], fit['r2']))
      P.xlabel('DPE [p/su]')
      P.ylabel('time step (T)')
      P.xticks(ticks=N.arange(0, max(xs) + 1, max(xs) / 10))
      yticks = N.arange(0, 25001, 5000)
      lab = ['0' if not n else str(n // 1000) + 'k' for n in yticks]
      P.yticks(yticks, labels=lab)
      P.xlim(0.05, 0.1)
      P.ylim(0, 25001)
      self.axes.margins(x=0, y=0)

      # set
      P.show()
      return

    if 'survive' == self.action:
      # prepare
      magenta = make_colormap(['#ffffff', '#8a008b', '#8a008b',
                               '#8a008b', '#8a008b', '#8a008b', '#520051'])
      blue = make_colormap(['#ffffff', '#65a4d3', '#5e73cb',
                            '#3c57aa', '#013583', '#002a75'])
      m = [[0 for i in range(101)] for j in range(30)]
      top = 0
      for moment in data:
        i = int(moment['dpe'] * 1000)
        for item in moment['est']: # for dbscan, change 'est' to 'dbscan'
          y = item[2]
          m[item[0]][i] = y
          if top < y:
            top = y
      cs = blue

      # plot
      p = P.imshow(m, cmap=cs, interpolation='none')

      # periphery
      P.xlabel('DPE [p/su]')
      P.ylabel('survived fraction time')
      P.xticks(N.arange(0, 101, 10), labels=N.arange(0, 0.11, 0.01))
      P.yticks(N.arange(0, 31, 6), labels=[0, 0.2, 0.4, 0.6, 0.8, 1])
      self.axes.invert_yaxis()
      #self.axes.set_aspect(0.75)

      # set
      #cb = P.colorbar(p, fraction=0.0219, aspect=10, pad=0.01)
      cb = P.colorbar(cax=make_axes_locatable(self.axes)
        .append_axes('right', size='3%', pad=0.1))
      cb.set_label('fraction of runs', rotation=-90, va='bottom')
      cb.set_ticks(N.arange(0, top + 1, top // 5))
      cb.set_ticklabels([0, 0.2, 0.4, 0.6, 0.8, 1])
      P.show()
      return

    if 'emerge' == self.action:
      # prepare
      ks = [v[0] for v in data]
      es = [v[1] for v in data]
      ds = [v[2] for v in data]
      evs = {}
      for i, x in enumerate(ks):
        if x not in evs:
          evs[x] = []
        evs[x].append(es[i])
      dvs = {}
      for i, x in enumerate(ks):
        if x not in dvs:
          dvs[x] = []
        dvs[x].append(ds[i])
      xs = N.array(list(evs.keys()))
      xt = N.arange(0, max(xs) + 1, max(xs) / 10)

      # plot (observed counts, estimated)
      bp = P.boxplot(list(evs.values()),
                     manage_ticks=False,
                     positions=list(evs.keys()),
                     widths=[0.0005] * len(evs),
                     flierprops={'marker': '+',
                                 'markersize': 1,
                                 'markeredgecolor': '#ba59b3'},
                     whiskerprops={'linestyle': 'dashed'},
                     patch_artist=True,
                     zorder=0)
      for b in bp['boxes']:
        b.set_facecolor('#d471cc')
        b.set_edgecolor('#ba59b3')
      for c in bp['caps']: c.set_color('#ba59b3')
      for f in bp['fliers']: f.set_color('#ba59b3')
      for m in bp['medians']: m.set_color('w')
      for w in bp['whiskers']: w.set_color('#d471cc')

      # plot (observed counts, dbscan)
      P.scatter(ks, ds,
                label='dbscan counting',
                color='#4ea494',
                s=1,
                zorder=10)

      # plot (fit, estimated)
      efit = self.approximate_emerge(xs, [median(y) for y in evs.values()])
      P.plot(efit['x'], efit['y'],
             label='fit (est.)',
             color='#4b7d08',
             zorder=30)

      # plot (fit, dbscan)
      dfit = self.approximate_emerge(xs, [median(y) for y in dvs.values()])
      P.plot(dfit['x'], dfit['y'],
             label='fit (dbscan)',
             color='#72a336',
             zorder=20)

      # periphery
      P.text(0.02, 7, 'est.:\n   N(DPE) = {:.2f} * DPE - {:.2f}\n     R^2 = {:.2f}'
        .format(efit['a'], -efit['b'], efit['r2']))
      P.text(0.02, 5, 'dbscan:\n N(DPE) = {:.2f} * DPE - {:.2f}\n     R^2 = {:.2f}'
        .format(dfit['a'], -dfit['b'], dfit['r2']))
      P.xlabel('DPE [p/su]')
      P.ylabel('number of structures (N)')
      P.xticks(ticks=N.arange(0, max(xs) + 1, max(xs) / 10))
      P.yticks(N.arange(10))
      #P.xlim(0.05, 0.1)
      #P.ylim(0, 25001)
      self.axes.margins(x=0, y=0)
      self.axes.set_aspect(0.004)
      P.rcParams['legend.handlelength'] = 0.5

      # set
      P.legend()
      P.show()
      return


## Experiment 5: "size & noise" ###############################################
#
#           | median | size   | lifetime      | noise
# ----------+--------+--------+---------------+--------------------------------
# PLOT TYPE |              area               | area
#           |        box-and-whiskers         |
#           |              line               |
# ----------+---------------------------------+--------------------------------
# when   IN | moment of change                | moment of change for each noise
# what      | tick, type of change, cell size | tick, type of change
# ----------+-----------------+---------------+--------------------------------
# x     OUT | cell size       | tick of death | noise
# y         | absolute freq   | relative freq | tick of death

class Exp5(Exp):

  id = 5
  actions = ['median', 'size', 'lifetime', 'noise']

  def __init__(self, args):
    super().__init__(args, False)
    self.check_action()

    # slight ickiness: input files must be prepended with a line with the DPE
    # input={dpe(str): [lines(str)]}
    input = {}
    for arg in self.inputs:
      with open(arg) as f:
        lines = f.read().splitlines()
        dpe = lines[0]
        if dpe not in input:
          input[dpe] = []
        input[dpe] += lines[1:]
    self.input = input

  def collect(self):
    input = self.input

    # data={method(str): {dpe(str):
    #   noise? {noise(int): {datum_key(str): datum_val(int or str)}}
    #   else?  {datum_key(str): datum_val(int, str, or list)}}}
    data = {'est': {}, 'dbscan': {}}

    if self.action in ['median', 'size', 'lifetime']:
      for dpe in input.keys():
        data['est'][dpe] = []
        data['dbscan'][dpe] = []

      for dpe, lines in input.items():
        for line in lines:
          e, d = line.split(': ')[1].split('; ')
          e = e.split(', ')
          d = d.split(', ')
          head = e[0].split()
          e[0] = ' '.join(head[3:])
          data['est'][dpe].append({
            'when': int(head[0]),
            'how': head[1][0],
            'size_counts': [list(map(int, sc.split())) for sc in e]
          })
          head = d[0].split()
          d[0] = ' '.join(head[3:])
          data['dbscan'][dpe].append({
            'when': int(head[0]),
            'how': head[1][0],
            'size_counts': [list(map(int, sc.split())) for sc in d]
          })

      self.data = data
      return

    if 'noise' == self.action:
      for dpe in input.keys():
        data['est'][dpe] = {}
        data['dbscan'][dpe] = {}

      for dpe, lines in input.items():
        for line in lines:
          for moment in line.split(': ')[1].split('; '):
            words = moment.split()
            noise = int(words[0])
            if noise not in data['est'][dpe]:
              data['est'][dpe][noise] = []
            data['est'][dpe][noise].append({
              'when': int(words[1]),
              'how': words[2][0]
            })
            if noise not in data['dbscan'][dpe]:
              data['dbscan'][dpe][noise] = []
            data['dbscan'][dpe][noise].append({
              'when': int(words[4]),
              'how': words[5][0]
            })

      self.data = data
      return

  def refine(self):
    input = self.input
    data = self.data

    # refined={method(str): {dpe(str): median?   {size(int): count(int)}
    #                                  size?     {size(int): count(int)
    #                                  lifetime? {tick(int): count(int)}
    #                                  noise?    {noise(): quartiles(list)}}}
    refined = {'est': {}, 'dbscan': {}}

    if 'median' == self.action:
      for dpe in input.keys():
        refined['est'][dpe] = [0 for i in range(100)]
        refined['dbscan'][dpe] = [0 for i in range(100)]
      for dpe, runs in self.data['est'].items():
        for run in runs:
          med = find_median(run['size_counts'])
          refined['est'][dpe][med] += 1
      for dpe, runs in self.data['dbscan'].items():
        for run in runs:
          med = find_median(run['size_counts'])
          refined['dbscan'][dpe][med] += 1

      self.refined = refined
      return

    if 'size' == self.action:
      for dpe in input.keys():
        refined['est'][dpe] = [0 for i in range(100)]
        refined['dbscan'][dpe] = [0 for i in range(100)]
      for dpe, runs in data['est'].items():
        for run in runs:
          for sc in run['size_counts']:
            refined['est'][dpe][sc[0]] += sc[1]
      for dpe, runs in self.data['dbscan'].items():
        for run in runs:
          for sc in run['size_counts']:
            refined['dbscan'][dpe][sc[0]] += sc[1]

      self.refined = refined
      return

    if 'lifetime' == self.action:
      for method in ['est', 'dbscan']:
        for dpe in input.keys():
          refined[method][dpe] = [0 for i in range(101)]
        for dpe, runs in data[method].items():
          bins = N.append(N.arange(0, 25000, 250), [25000])
          whens = [x['when'] for x in runs]
          occur = discretise(bins, whens)
          for o in occur.values():
            refined[method][dpe][o[0]] = o[1]

      self.refined = refined
      return

    if 'noise' == self.action:
      for dpe in input.keys():
        refined['est'][dpe] = {}
        refined['dbscan'][dpe] = {}
      for dpe, moments in self.data['est'].items():
        for noise, changes in moments.items():
          ticks = [x['when'] for x in changes]
          refined['est'][dpe][noise] = quantiles(ticks, n=4)
      for dpe, moments in self.data['dbscan'].items():
        for noise, changes in moments.items():
          ticks = [x['when'] for x in changes]
          refined['dbscan'][dpe][noise] = quantiles(ticks, n=4)

      self.refined = refined
      return

  def plot(self):
    action = self.action
    dpes = list(self.input.keys())

    # prepare
    red = '#a32c2d'
    green = '#4b7d08'
    blue = '#3c56aa'
    redl = '#daaaab'
    greenl = '#b4c999'
    bluel = '#b1bbdd'
    data = {}
    for dpe in dpes:
      data[dpe] = self.refined['est'][dpe] # for dbscan, change 'est' to 'dbscan'

    if self.action in ['median', 'size', 'lifetime']:
      # more prep
      index = 0

      # find global max
      top = 0
      for ys in data.values():
        m = max(ys)
        if top < m:
          top = m

      # prepare and plot for each dpe
      for dpe, ys in data.items():
        # find max for this dpe
        peak = [0, 0]
        for x, y in enumerate(ys):
          if peak[1] < y:
            peak[0] = x
            peak[1] = y

        # flatten frequencies into one list for easy detection of quartiles
        freq = []
        for i, n in enumerate(ys):
          freq += [i] * n # slow for 'size'

        c = red if '4' in dpe else green if '5' in dpe else blue
        cl = redl if '4' in dpe else greenl if '5' in dpe else bluel
        wid = 50 if 'lifetime' == action else \
              125000 if 'size' == action else \
              12.5
        pos = top + 2 * wid + 2 * index * wid
        index += 1

        # for 'lifetime', convert counts into percentages
        if 'lifetime' == action:
          s = sum(ys)
          for i, n in enumerate(ys):
            ys[i] = n / s
          peak = [peak[0], peak[1] / s]
          wid = wid / s
          pos = pos / s - wid * 3

        # plot
        P.fill_between(N.arange(0, len(ys)), ys,
                       color=c,
                       alpha=0.4,
                       zorder=10)
        P.plot([peak[0], peak[0]], [0, peak[1]],
               color='#808080',
               linestyle='dashed',
               linewidth=0.5,
               zorder=20)
        bp = P.boxplot(freq,
                       vert=False,
                       positions=[pos],
                       widths=[wid],
                       whis=(0, 100),
                       manage_ticks=False,
                       showfliers=False,
                       whiskerprops={'linestyle': 'solid'},
                       patch_artist=True,
                       zorder=0)
        for x in bp['boxes']: x.set_facecolor(cl); x.set_edgecolor(c)
        for x in bp['caps']: x.set_color(c)
        for x in bp['medians']: x.set_color(c)
        for x in bp['whiskers']: x.set_color(cl)

      # periphery
      if   'median'   == action: P.xlabel('median cell size per run')
      elif 'size'     == action: P.xlabel('cell size in all time steps')
      elif 'lifetime' == action: P.xlabel('cell lifetime [time step]')
      if   'lifetime' == action: P.ylabel('relative frequency in all runs')
      else:                      P.ylabel('absolute frequency in all runs')
      if 'lifetime' == action:
        P.xticks(ticks=N.arange(0, len(ys), 20),
                 labels=N.arange(0, 25001, 5000))
        P.yticks(ticks=N.arange(0, 1.01, 0.1))
      else:
        P.xlim(-2, 65)
      if 'lifetime' == action:
        self.axes.margins(x=0.01, y=0)
        self.axes.set_aspect(35)
      else:
        self.axes.set_aspect(2 / wid)

      # set
      P.grid(axis='y', linestyle='dotted')
      P.show()
      return

    if 'noise' == action:
      # more prep
      xs = N.arange(0, len(list(data.values())[0]))

      # plot
      for dpe, yss in data.items():
        first = [ys[0] for ys in yss.values()]
        med   = [ys[1] for ys in yss.values()]
        third = [ys[2] for ys in yss.values()]
        c = red if '4' in dpe else green if '5' in dpe else blue
        P.fill_between(N.arange(0, len(med)), third, first, color=c, alpha=0.3)
        P.plot(xs, med, label='DPE = {} p/su'.format(dpe), color=c, alpha=0.4)

      # periphery
      P.xlabel('standard deviation of noise [degree]')
      P.ylabel('cell lifetime')
      P.xticks(ticks=N.arange(0, len(xs) + 1, 2), labels=N.arange(0, 91, 10))
      self.axes.margins(x=0, y=0)
      self.axes.set_aspect(0.0003)
      P.rcParams['legend.handlelength'] = 0.5

      # set
      P.grid(axis='y', linestyle='dotted')
      P.legend()
      P.show()
      return


## Experiment 6: "param sweep" ################################################
#
# plot type:
# - heatmap
#
# in:
# - when: alpha @ +3 increments and beta @ +1 increments
# - what: dhi
#
# out:
# - x: alpha
# - y: beta
# - z: dhi

class Exp6(Exp):

  id = 6
  actions = []

  def __init__(self, args):
    super().__init__(args)

  def collect(self):
    input = self.input
    data = {}

    for line in input:
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

    self.data = data

  def refine(self):
    data = self.data
    refined = []

    for datum in data.values():
      refined.append([datum['alpha'], datum['beta'],
                     sum(datum['dhi']) / len(datum['dhi'])])

    self.refined = refined

  def plot(self):
    data = self.refined

    # prepare
    cs = make_colormap(["#103080", "#4040ff", "#00c0ff",
                        "#00c0c0", "#ffff00", "#ff0000",
                        "#800000", "#400000", "#000000"])
    m = [[0 for i in range(121)] for j in range(121)]
    for datum in data:
      m[datum[1] + 60][(datum[0] + 180) // 3] = datum[2]

    # plot
    P.imshow(m, cmap=cs, interpolation='nearest')

    # periphery
    P.xlabel('alpha [degree]')
    P.ylabel('beta [degree]')
    P.xticks(ticks=N.arange(0, 121, 20), labels=N.arange(-180, 181, 60))
    P.yticks(ticks=N.arange(0, 121, 20), labels=N.arange(-60, 61, 20))
    self.axes.invert_yaxis()
    #self.axes.margins(x=0, y=0)
    self.axes.set_aspect(1)

    # set
    cb = P.colorbar(cax=make_axes_locatable(self.axes)
      .append_axes('right', size='5%', pad=0.1))
    cb.set_label('density homogeneity index', rotation=-90, va='bottom')
    P.show()
    return


## run ########################################################################

exp_table = [None, Exp1, Exp2, Exp3, Exp4, Exp5, Exp6, Perf]
usage_message = 'EXP_NUM [EXP_ACTION] DATA_FILES'
P.rcParams.update({
  'figure.autolayout': True,
  'font.family': 'Liberation Mono',
  'toolbar': 'None'
})

if (2 > len(sys.argv)):
  usage(usage_message, True)
  exit(-1)
e = sys.argv[1]
if e not in ['1', '2', '3', '4', '5', '6', '7']:
  usage(usage_message, True)
  exit(-1)

exp = exp_table[int(e)](sys.argv[2:])
exp.collect()
exp.refine()
exp.plot()
exit(0)

