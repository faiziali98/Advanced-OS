set title "file size vs time taken (Local Server)"
set xlabel "file size"
set ylabel "time taken(s)"
plot "timeLocal.txt" title"" with line,"timeGlobal.txt" title"" with line  
set term png
set output "plot1.png"
replot
