set style line 1 lc rgb '#0060ad' lt 1 lw 2   # --- blue
set style line 2 lc rgb '#dd181f' lt 1 lw 2   # --- red
set style line 3 lc rgb '#00ff00' lt 1 lw 2   # --- green
set yrange [-1.2:1.2]
plot 'ncc.dat' index 0 with linespoints ls 1, \
     ''        index 1 with lines ls 2, \
     ''        index 2 with lines ls 3
