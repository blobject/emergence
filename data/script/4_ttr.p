set title 'exp 4: survival'
set size 1,0.6

set xtics 1000
set xrange [5000:10200]

plot f using 2 title 'est.' with points pointtype 13 pointsize 0.75

#set xtics 1000
#set xrange [3600:10200]
#
#plot f using 2 title 'est.' with points pointtype 13 pointsize 0.75, \
#     f using 3 title 'dbscan' with points pointtype 13 pointsize 0.75

