set title 'exp 6: param sweep'
set view map
set size square
set xrange [-180:180]
set yrange [-60:60]
set xtics 60
set ytics 10
set palette defined (0 "#103080",\
                     1 "#4040ff",\
                     2 "#00c0ff",\
                     3 "#00c0c0",\
                     4 "#ffff00",\
                     5 "#ff0000",\
                     6 "#800000",\
                     7 "#400000",\
                     8 "#000000")

splot f using 1:2:3 with image

