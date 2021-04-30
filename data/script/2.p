set title 'exp 2: stability'
set xtics 1000

est_cell = '#00ff00'
est_spore = '#c0c000'
dbscan_cell = '#00aa00'
dbscan_spore = '#707000'

plot f using 2 title 'est. cells' with lines smooth sbezier, \
     f using 4 title 'dbscan cells' with lines smooth sbezier, \
     f using 3 title 'est. spores' with lines smooth sbezier, \
     f using 5 title 'dbscan spores' with lines smooth sbezier

