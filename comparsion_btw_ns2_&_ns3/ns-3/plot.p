set terminal png
unset label
set autoscale
set title ""
set xlabel "Time (s)"
set ylabel "Throughput (Mbps)"
set xrange [0:100]
set output "firstbulk.png"
plot "first/TP-first-bulksend.pcap-6-2.plotme" using 1:2 title "SFQ in ns-3" with lines lc rgb "red"

set output "secondbulk.png"
plot "second/TP-second-bulksend.pcap-51-2.plotme" using 1:2 title "SFQ in ns-3" with lines lc rgb "red"

set output "thirdmix.png"
plot "third/TP-third-mix.pcap-8-2.plotme" using 1:2 title "SFQ in ns-3" with lines lc rgb "red"

set output "thirdmixtcpudp.png"
plot "third/TP-third-mix.pcap-8-2TCP.plotme" using  1:2 title "SFQ with TCP in ns-3" with lines lc rgb "red",\
"third/TP-third-mix.pcap-8-2UDP.plotme" using  1:2 title "SFQ with UDP in ns-3" with lines lc rgb "green"
