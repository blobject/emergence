set title 'exp 4: survive'
set size 1,0.6
set xrange [0:0.1]
set yrange [-0.05:1.05]
set xtics 0.01
set ytics 0.2
#blue
set palette defined (0 '#f0f0ff',\
                     1 '#a8b8ff',\
                     2 '#667ad3',\
                     3 '#3c56aa',\
                     4 '#254696',\
                     5 '#013583')
# magenta
#set palette defined (0 '#fff0ff',\
#                     1 '#d471cc',\
#                     2 '#ba59b3',\
#                     3 '#91328c',\
#                     4 '#7d1c79',\
#                     5 '#6a0067')

plot f using 1:2:3 notitle with points pointtype 13 pointsize 1 palette
#plot f using 1:4:5 notitle with points pointtype 13 pointsize 1 palette

