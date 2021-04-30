set title 'exp 1: occupancy'
set style data histogram
set style fill solid
set style histogram cluster gap 1
set yrange [0:100]

first = '#c0c0c0'
last = '#000000'

plot f using 2:xtic(1) title 't=0  ' linecolor rgb first, \
     f using 3         title 't=150' linecolor rgb last
#     f using 3         title 't=60', \
#     f using 4         title 't=90', \
#     f using 5         title 't=180', \
#     f using 6         title 't=400', \
#     f using 7         title 't=700' linecolor rgb last

