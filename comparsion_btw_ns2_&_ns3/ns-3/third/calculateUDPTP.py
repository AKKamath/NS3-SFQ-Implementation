# This program is used to plot throughput calculation from pcap file 

import sys
import os

for file_name in sys.argv[1:]:
    os.system("sudo tshark -r " + file_name +" -T fields -e frame.time_relative -e frame.len -e ip.dst udp.port == 50001  >" + file_name[0:file_name.rindex('.')] + ".csv")
    f = open( 'TP-' + file_name[0:file_name.rindex('.')] + 'UDP.plotme', 'w')
    old_time = 0
    new_time =0
    val = 0
    p = 0.0
    with open(file_name[0:file_name.rindex('.')] + '.csv') as fq:
        fp = fq.readlines()
        for line in fp:
            if str(line.split('\t')[2]) == "10.0.9.2\n":
                new_time = float(line.split('\t')[0])
                val += float(line.split('\t')[1])
                if (new_time - old_time > 0.1):
                    val = val/(new_time - old_time)
                    f.write(str(p) + " " + str(val * 8.0/(1000.0 * 1000.0)) + "\n")
                    p += (new_time - old_time)
                    old_time = new_time
                    val = 0
    f.close()
    os.remove(file_name[0:file_name.rindex('.')] + '.csv')
