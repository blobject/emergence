set title 'exp 3: heat map'
set view map
set size square
set tics scale 0.5
set xrange [-1:35]
set yrange [-0.8:35]
set xtics 5
set ytics 5
# back
set palette defined (0 '#ffffff',\
                     1 '#e0e0e0',\
                     2 '#d0d0d0',\
                     3 '#b8b8b8',\
                     4 '#a0a0a0',\
                     5 '#888888',\
                     6 '#707070',\
                     7 '#505050',\
                     8 '#202020')
# green
#set palette defined (0 '#ffffff',\
#                     1 '#72a336',\
#                     2 '#5a8b1c',\
#                     3 '#538413',\
#                     4 '#4b7d07',\
#                     5 '#427400',\
#                     6 '#346600',\
#                     7 '#0f4200',\
#                     8 '#073500')
# brown
#set palette defined (0 '#ffffff',\
#                     1 '#d47f3e',\
#                     2 '#ba6827',\
#                     3 '#b16120',\
#                     4 '#a95a19',\
#                     5 '#9f510f',\
#                     6 '#8f4400',\
#                     7 '#662100',\
#                     8 '#581200')
# magenta
#set palette defined (0 '#fff8ff',\
#                     1 '#6a0067',\
#                     2 '#ffffff',\
#                     3 '#ffffff',\
#                     4 '#ffffff',\
#                     5 '#ffffff',\
#                     6 '#ffffff',\
#                     7 '#ffffff',\
#                     8 '#ffffff')
# blue
#set palette defined (0 '#ffffff',\
#                     1 '#7f91ed',\
#                     2 '#657ad3',\
#                     3 '#5e73cb',\
#                     4 '#566cc3',\
#                     5 '#4c64ba',\
#                     6 '#3c57aa',\
#                     7 '#013583',\
#                     8 '#002a75')
# yellow
#set palette defined (0 '#fff8e0',\
#                     1 '#ffe690',\
#                     2 '#ffcc78',\
#                     3 '#ba8d3b',\
#                     4 '#a07624',\
#                     5 '#ffffff',\
#                     6 '#ffffff',\
#                     7 '#ffffff',\
#                     8 '#ffffff')

splot f using 1:2:3 notitle with points pointtype 5 pointsize 1.35 palette

