set title "file size vs time taken (Direct vs proxies)"
set xlabel "file size"
set ylabel "time taken(s)"
plot "timeDirect.txt" title"" with line,"timeProxyGlobal.txt" title"" with line,"timeProxyLocal.txt" title"" with line 
set term png
set output "plot1.png"
replot
