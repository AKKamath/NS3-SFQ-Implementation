# Implementation of Stochastic Fair Queueing in NS-3
## Course Code: CO300

### Overview

Stochastic Fair Queueing is an attempt to distribute network usage fairly among different source-destination pairs of messages. For absolute fairness, a hash between a source-destination pair and a queue is created, and round-robin is employed among the queues. However, this consumes a lot of resources and is not practical. Stochastic Fair Queueing employs an approxiate hash, which is more viable. The hash function is perturbed periodically to maintain fairness.
### SFQ Example
An example programme for SFQ is provided in  
``src/traffic-control/examples/sfq-example.cc``  
and should be executed as  
``./waf --run "sfq-example"``   

### References:

[1] Kuznetsov, Alexey. “Stochastic Fairness Queueing.” Github, github.com/torvalds/linux/blob/master/net/sched/sch_sfq.c

[2] Paul E. McKenney "Stochastic Fairness Queuing",	"Interworking: Research and Experience", v.2, 1991, p.113-131.
