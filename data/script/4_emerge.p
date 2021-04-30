set title 'exp 4: survive'
set key left top
set size 1,0.6
set xrange [0:10200]
set yrange [0:10]
set xtics 1000
set ytics 1

plot f using 2 title 'est.' with points pointtype 13 pointsize 0.75, \
     f using 3 title 'dbscan' with points pointtype 13 pointsize 0.75

