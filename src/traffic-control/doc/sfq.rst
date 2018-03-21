.. include:: replace.txt
.. highlight:: cpp
.. highlight:: bash

Sfq queue disc
------------------

This chapter describes the Sfq ([1]_) queue disc implementation in |ns3|.

The Stochastic Fair Queuing (SFQ) algorithm is a Queue Management algorithm.
SFQ is an attempt to distribute network usage fairly among different 
source-destination pairs of messages. For absolute fairness, a hash 
between a source-destination pair and a queue is created, and round-robin 
is employed among the queues. However, this consumes a lot of resources and 
is not practical. Stochastic Fair Queueing employs an approxiate hash, which 
is more viable. The hash function is perturbed periodically to maintain 
fairness. Each queue is served according to its current allotment, a positive 
allotment means that the queue may be served, if it is next in order, while a 
negative allotment means that the next queue in the list should be taken.

Model Description
*****************

The source code for the Sfq queue disc is located in the directory
``src/traffic-control/model`` and consists of 2 files `sfq-queue-disc.h`
and `sfq-queue-disc.cc` defining a SfqQueueDisc class and a helper
SfqFlow class. The code was ported to |ns3| based on Linux kernel code
implemented by Alexey Kuznetsov as well as the ns-2 code for Sfq.

* class :cpp:class:`SfqQueueDisc`: This class implements the main Sfq algorithm:

  * ``SfqQueueDisc::DoEnqueue ()``: This routine uses the configured packet 
filters to classify the given packet into an appropriate queue. If the filters 
are unable to classify the packet, the packet is added to a spare queue. If the
queue is an empty slot, it is added to the beginning of the list of queues, and
its allotment is initiated to the configured quantum. Otherwise, the queue is
 left in its current queue list. Finally, the total number of enqueued packets 
is compared with the configured limit, and if it is above this value (which can
 happen since a packet was just enqueued), packets are dropped from the head of
 the queue with the largest current byte count until the number of dropped 
packets reaches the configured drop batch size or the backlog of the queue has
 been halved. Note that this in most cases means that the packet that was just
 enqueued is not among the packets that get dropped, which may even be from a 
different queue.

  * ``SfqQueueDisc::DoDequeue ()``: The first task performed by this routine
is selecting a queue from which to dequeue a packet. The scheduler looks at
the front of the list, if the current queue has a negative allotment, it's
allotment is increased by quantum and sent to the back. Otherwise, that queue
is selected for dequeue. After having selected a queue from which to dequeue a
packet, the Pfifo algorithm is invoked on that queue. As a result of this,
one packets is discarded from the head of the selected queue, before the 
packet that should be dequeued is returned (or nothing is returned if the 
queue is or becomes empty while being handled by the Pfifo algorithm). 
Finally, if the Pfifo algorithm does not return a packet, then the queue must 
be empty, and the scheduler removes it from the list, marking it as an empty 
slot to be added back (as a new queue) the next time a packet for that queue 
arrives. Then (since no packet was available for dequeue), the whole dequeue 
process is restarted from the beginning. If, instead, the scheduler did get a 
packet back from the Pfifo algorithm, it subtracts the size of the packet 
from the byte allotment for the selected queue and returns the packet as the 
result of the dequeue operation.

  * ``SfqQueueDisc::SfqDrop ()``: This routine is invoked by 
``SfqQueueDisc::DoEnqueue()`` to drop packets from the head of the queue with 
the largest current byte count. This routine keeps dropping packets until the 
number of dropped packets reaches the configured drop batch size or the 
backlog of the queue has been halved.

* class :cpp:class:`SfqFlow`: This class implements a flow queue, by keeping 
its current status (an empty slot, or in use) and its current allotment.

In Linux, by default, packet classification is done by hashing (using a 
Jenkins hash function) and taking the hash value modulo the number of queues.
The hash is salted by modulo addition of a random value selected at regular 
intervals, in order to perturb the hash function, reducing the chances of 
the same collision consistently occuring. Alternatively, any other packet 
filter can be configured.
In |ns3|, at least one packet filter must be added to an Sfq queue disc.


References
==========

.. [1] Kuznetsov, Alexey. “Stochastic Fairness Queueing.” Github, github.com/torvalds/linux/blob/master/net/sched/sch_sfq.c

.. [2] Paul E. McKenney "Stochastic Fairness Queuing",	"Interworking: Research and Experience", v.2, 1991, p.113-131.

Attributes
==========

The key attributes that the SfqQueue class holds include the following:

* ``Packet limit:`` The limit on the maximum number of packets stored by Sfq.
* ``Flows:`` The number of flow queues managed by Sfq.

Note that the quantum, i.e., the number of bytes each queue gets to dequeue on
each round of the scheduling algorithm, is set by default to the MTU size of 
the device (at initialisation time). The ``SfqQueueDisc::SetQuantum ()`` method
can be used (at any time) to configure a different value.

Examples
========

A typical usage pattern is to create a traffic control helper and to 
configure type and attributes of queue disc and filters from the helper. 
For example, Sfq can be configured as follows:

.. sourcecode:: cpp

  TrafficControlHelper tch;
  uint16_t handle = tch.SetRootQueueDisc ("ns3::SfqQueueDisc", 
"DropBatchSize", UintegerValue (1));
  tch.AddPacketFilter (handle, "ns3::SfqIpv4PacketFilter", 
"Perturbation_Time", UintegerValue (100));
  tch.AddPacketFilter (handle, "ns3::SfqIpv6PacketFilter");
  QueueDiscContainer qdiscs = tch.Install (devices);

