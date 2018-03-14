set terminal png
unset label
set autoscale
set title ""
set xlabel "Time (s)"
set ylabel "Throughput (Mbps)"
set xrange [0:100]
set output "firstbulk.png"
plot "first/Throughput.plotme" using 1:2 title "SFQ in ns-2" with lines lc rgb "red"

set output "secondbulk.png"
plot "second/Throughput.plotme" using 1:2 title "SFQ in ns-2" with lines lc rgb "red"

set output "thirdmix.png"
plot "third/Throughput.plotme" using 1:2 title "SFQ in ns-2" with lines lc rgb "red"

set output "thirdmixtcpudp.png"
plot "third/ThroughputTCP.plotme" using  1:2 title "SFQ with TCP in ns-2" with lines lc rgb "red",\
"third/ThroughputUDP.plotme" using  1:2 title "SFQ with UDP in ns-2" with lines lc rgb "green"
